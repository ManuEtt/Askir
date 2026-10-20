#include "Inventory/AskirInventoryComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Characters/AskirNPC.h"
#include "NPC/AskirDailyRoutineComponent.h"
#include "NPC/AskirNPCInteractionComponent.h"
#include "NPC/AskirDialogueComponent.h"
#include "AI/AskirCivilianAIController.h"
#include "AI/AskirEnemyAIController.h"
#include "AIController.h"
#include "Combat/AskirCombatComponent.h"
#include "Combat/AskirCombatTargetPolicy.h"
#include "Quest/AskirQuestComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AskirAnimationStateComponent.h"
#include "Animation/AskirAnimationPlaybackComponent.h"
#include "Magic/AskirMagicComponent.h"
#include "Combat/AskirGuardComponent.h"

AAskirNPC::AAskirNPC()
{
    PrimaryActorTick.bCanEverTick = true;
    if(AnimationPlayback)AnimationPlayback->ConfigureCharacterVisual(TEXT("Guard"));
    CreateDefaultSubobject<UAskirDailyRoutineComponent>(TEXT("DailyRoutine"));
    CreateDefaultSubobject<UAskirNPCInteractionComponent>(TEXT("Interaction"));

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AAskirNPC::BeginPlay()
{
    Super::BeginPlay();
    HomeLocation = GetActorLocation();
    ConfigureCombatStyle();
    if (CombatComponent)
    {
        CombatComponent->OnDeath.AddDynamic(this, &AAskirNPC::HandleNPCDeath);
        CombatComponent->OnDamageTaken.AddDynamic(this, &AAskirNPC::HandleNPCDamage);
    }
}

void AAskirNPC::ConfigureCombatStyle()
{
    // Movement speed is set once; combat multipliers are evaluated at attack time.
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        const float BaseSpeed = Movement->MaxWalkSpeed;
        if (CombatStyle == EAskirEnemyStyle::Bruiser) Movement->MaxWalkSpeed = BaseSpeed * 0.75f;
        else if (CombatStyle == EAskirEnemyStyle::Skirmisher) Movement->MaxWalkSpeed = BaseSpeed * 1.25f;
    }
}

void AAskirNPC::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if(IsWorldTravelPending())
    {
        CancelPendingAttack();AbortPendingAttack();
        if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();
        return;
    }
    if (!bHostile || !bEncounterActive || IsDead() || !GetWorld()) return;
    // A dedicated controller owns decisions and attack timing; do not run two AIs.
    if(Cast<AAskirEnemyAIController>(GetController()))return;
    if (IsStaggered()) return;

    const float Now = GetWorld()->GetTimeSeconds();
    if (PendingAttackTarget.IsValid())
    {
        if (Now >= AttackResolveTime) ResolvePendingAttack(Now);
        return;
    }
    if (Now < NextDecisionTime) return;
    NextDecisionTime = Now + FMath::Max(0.05f, DecisionInterval);
    UpdateAI();
}

void AAskirNPC::SetEncounterActive(bool bActive)
{
    bEncounterActive=bActive && !IsDead();
    if(CombatComponent)CombatComponent->SetDamageEnabled(bEncounterActive);
    SetActorHiddenInGame(!bEncounterActive);
    SetActorEnableCollision(bEncounterActive);
    if(!bEncounterActive)
    {
        if(MagicComponent)MagicComponent->CancelCasting();
        CancelPendingAttack();
        AbortPendingAttack();
        StopBlock();
        if(AAIController* AI=Cast<AAIController>(GetController()))AI->StopMovement();
        GetCharacterMovement()->DisableMovement();
    }
    else if(GetCharacterMovement()->MovementMode==MOVE_None)
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AAskirNPC::RestoreEncounterState(bool bDefeated)
{
    CancelPendingAttack();
    AbortPendingAttack();
    StopBlock();
    if(GuardComponent)GuardComponent->RestoreStamina();
    SetLifeSpan(0.f);
    bRewardsGranted=bDefeated;
    StaggerEndTime=-1.f;
    NextStaggerAllowedTime=0.f;
    SearchEndTime=0.f;
    NextDecisionTime=0.f;
    NextAttackTime=GetWorld()?GetWorld()->GetTimeSeconds()+FMath::Max(0.f,AttackRecovery):0.f;
    AIState=EAskirNPCState::Idle;
    if(CombatComponent)CombatComponent->RestoreHealth(bDefeated?0.f:CombatComponent->GetMaxHealth());
    if(MagicComponent){FAskirMagicSnapshot Snapshot;Snapshot.Mana=MagicComponent->MaxMana;MagicComponent->RestoreSnapshot(Snapshot);}
    if(AnimationState)AnimationState->ResetAfterLoad(bDefeated);
    if(AnimationPlayback)AnimationPlayback->RestorePlaybackAfterLoad();
    if(!bDefeated)GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SetEncounterActive(!bDefeated);
}

bool AAskirNPC::CanSeeTarget(const AActor* Target) const
{
    if (!Target || !GetWorld()) return false;
    const FVector EyePosition = GetActorLocation() + FVector(0.f, 0.f, BaseEyeHeight);
    const FVector TargetPosition = Target->GetActorLocation() + FVector(0.f, 0.f, 50.f);
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(AskirNPCSight), false, this);
    const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, EyePosition, TargetPosition, ECC_Visibility, Params);
    return !bBlocked || Hit.GetActor() == Target;
}

void AAskirNPC::SetAIState(EAskirNPCState NewState)
{
    if (AIState == NewState) return;
    AIState = NewState;
    if (NewState != EAskirNPCState::Pursuing) CancelPendingAttack();
    if (AAIController* AI = Cast<AAIController>(GetController())) AI->StopMovement();
}

void AAskirNPC::UpdateAI()
{
    AAIController* AI = Cast<AAIController>(GetController());
    if (!AI) return;

    AActor* Target = UGameplayStatics::GetPlayerPawn(this, 0);
    if (const AAskirCharacter* CharacterTarget = Cast<AAskirCharacter>(Target))
    {
        if (CharacterTarget->IsDead()) Target = nullptr;
    }

    const float Now = GetWorld()->GetTimeSeconds();
    const float DistanceFromHome = FVector::Dist(GetActorLocation(), HomeLocation);
    const float DistanceToTarget = Target ? FVector::Dist(GetActorLocation(), Target->GetActorLocation()) : TNumericLimits<float>::Max();
    const bool bInSightRange = Target && DistanceToTarget <= (AIState == EAskirNPCState::Pursuing ? LoseTargetRadius : DetectionRadius);
    const bool bVisible = bInSightRange && CanSeeTarget(Target);
    const bool bWithinLeash = DistanceFromHome <= MaxChaseDistance;

    // Returning enemies finish their reset before reacquiring the same distant player.
    if(AIState==EAskirNPCState::Returning)
    {
        if(DistanceFromHome>90.f)AI->MoveToLocation(HomeLocation,60.f);
        else { AI->StopMovement(); SetAIState(EAskirNPCState::Idle); }
        return;
    }
    const bool bTargetWithinLeash=Target && FVector::DistSquared(Target->GetActorLocation(),HomeLocation)<=FMath::Square(FMath::Max(0.f,MaxChaseDistance));

    if (bVisible && bWithinLeash && bTargetWithinLeash)
    {
        LastKnownTargetLocation = Target->GetActorLocation();
        SetAIState(EAskirNPCState::Pursuing);
        const float DesiredDistance = CombatStyle == EAskirEnemyStyle::Skirmisher
            ? FMath::Max(40.f, AttackDistance - SkirmisherRetreatDistance)
            : AttackDistance;
        if (DistanceToTarget > DesiredDistance)
        {
            AI->MoveToActor(Target, FMath::Max(30.f, DesiredDistance * 0.75f));
        }
        else
        {
            AI->StopMovement();
            const FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
            if (!Direction.IsNearlyZero()) SetActorRotation(Direction.Rotation());
            if (Now >= NextAttackTime)
            {
                const float IntervalMultiplier = CombatStyle == EAskirEnemyStyle::Bruiser
                    ? BruiserIntervalMultiplier
                    : (CombatStyle == EAskirEnemyStyle::Skirmisher ? SkirmisherIntervalMultiplier : 1.f);
                const float WindupMultiplier = CombatStyle == EAskirEnemyStyle::Bruiser ? 1.5f : 1.f;
                PendingAttackTarget = Target;
                AttackResolveTime = Now + FMath::Max(0.f, AttackWindup * WindupMultiplier);
                NextAttackTime = AttackResolveTime + FMath::Max(0.1f, AttackInterval * IntervalMultiplier) + AttackRecovery;
                OnAttackTelegraphed(); // Animation / sound can warn the player.
                if(AnimationState){if(CombatStyle==EAskirEnemyStyle::Bruiser)AnimationState->BeginHeavyAttack();else AnimationState->BeginLightAttack();}
            }
        }
        return;
    }

    if (AIState == EAskirNPCState::Pursuing && bWithinLeash && bTargetWithinLeash)
    {
        SearchEndTime = Now + SearchDuration;
        SetAIState(EAskirNPCState::Searching);
    }

    if (AIState == EAskirNPCState::Searching && Now < SearchEndTime && bWithinLeash && bTargetWithinLeash)
    {
        AI->MoveToLocation(LastKnownTargetLocation, 75.f);
        return;
    }

    if (AIState != EAskirNPCState::Idle) SetAIState(EAskirNPCState::Returning);
    if (FVector::DistSquared(GetActorLocation(), HomeLocation) > FMath::Square(90.f))
    {
        AI->MoveToLocation(HomeLocation, 60.f);
    }
    else
    {
        AI->StopMovement();
        SetAIState(EAskirNPCState::Idle);
    }
}

void AAskirNPC::CancelPendingAttack()
{
    if (PendingAttackTarget.IsValid()) OnAttackInterrupted();
    PendingAttackTarget.Reset();
    AttackResolveTime = -1.f;
}

void AAskirNPC::ResolvePendingAttack(float Now)
{
    AActor* Target = PendingAttackTarget.Get();
    // Resolution is not an interruption: clear without firing the interrupt event.
    PendingAttackTarget.Reset();
    AttackResolveTime = -1.f;
    if (!Target || !bEncounterActive || IsDead() || IsStaggered() || !GetWorld()) return;
    if(!AskirCombatTargetPolicy::CanDamage(this,Target)){OnAttackAvoided();return;}
    if (const AAskirCharacter* Character = Cast<AAskirCharacter>(Target))
    {
        if (Character->IsDead()) return;
    }
    // Revalidate at impact: dodging out of range or moving behind cover avoids damage.
    const float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (Distance > AttackDistance || !CanSeeTarget(Target)
        || FVector::DistSquared(GetActorLocation(),HomeLocation)>FMath::Square(FMath::Max(0.f,MaxChaseDistance))
        || FVector::DistSquared(Target->GetActorLocation(),HomeLocation)>FMath::Square(FMath::Max(0.f,MaxChaseDistance)))
    {
        OnAttackAvoided();
        return;
    }
    const FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    if (Direction.IsNearlyZero()) return;
    SetActorRotation(Direction.Rotation());
    const float DamageMultiplier = CombatStyle == EAskirEnemyStyle::Bruiser
        ? BruiserDamageMultiplier
        : (CombatStyle == EAskirEnemyStyle::Skirmisher ? SkirmisherDamageMultiplier : 1.f);
    if (UAskirCombatComponent* TargetCombat = Target->FindComponentByClass<UAskirCombatComponent>())
    {
        FAskirDamageInfo Damage;
        Damage.Damage = AttackDamage * DamageMultiplier;
        Damage.DamageCauser = this;
        Damage.HitLocation = Target->GetActorLocation();
        Damage.HitDirection = Direction;
        const float HealthBefore=TargetCombat->GetHealth();
        if (AAskirCharacter* Player = Cast<AAskirCharacter>(Target)) Player->ReceiveAskirDamage(Damage);
        else TargetCombat->ApplyDamage(Damage);
        if(TargetCombat->GetHealth()<HealthBefore)OnAttackLanded();
        else OnAttackAvoided();
    }
}

bool AAskirNPC::IsStaggered() const
{
    return GetWorld() && GetWorld()->GetTimeSeconds() < StaggerEndTime;
}

void AAskirNPC::HandleNPCDamage(const FAskirDamageInfo& DamageInfo)
{
    if (!bHostile || !GetWorld() || IsDead() || DamageInfo.Damage < StaggerDamageThreshold) return;
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now < NextStaggerAllowedTime || StaggerDuration <= 0.f) return;
    CancelPendingAttack(); // A strong hit interrupts the telegraphed attack.
    AbortPendingAttack(); // Also cancel the pawn timer used by a dedicated AI controller.
    StaggerEndTime = Now + StaggerDuration;
    NextStaggerAllowedTime = StaggerEndTime + StaggerImmunityDuration;
    NextAttackTime = FMath::Max(NextAttackTime, StaggerEndTime);
    if (AAIController* AI = Cast<AAIController>(GetController())) AI->StopMovement();
    OnStaggered(DamageInfo); // Blueprint animation or VFX hook, optional.
}

void AAskirNPC::HandleNPCDeath()
{
    // Latch before external delegates: quest/reward callbacks can re-enter gameplay.
    if(!IsDead() || bRewardsGranted)return;
    bRewardsGranted=true;
    AActor* Recipient=CombatComponent?CombatComponent->GetLastDamageCauser():nullptr;
    const APawn* RecipientPawn=Cast<APawn>(Recipient);
    if (!DefeatObjectiveID.IsNone() && RecipientPawn && RecipientPawn->IsPlayerControlled())
    {
        if (UAskirQuestComponent* Quests = Recipient->FindComponentByClass<UAskirQuestComponent>())
        {
            Quests->AdvanceObjective(DefeatObjectiveID, 1);
        }
    }
    // Grant rewards exactly once to the actor responsible for the killing blow.
    if (Recipient)
    {
        const int32 XP=FMath::Max(0,KillExperience);
        if (XP>0)
            if (UAskirProgressionComponent* Progression=Recipient->FindComponentByClass<UAskirProgressionComponent>())
                Progression->AwardExperience(XP);
        bool bItemGranted=false;
        if (GuaranteedDrop && GuaranteedDropQuantity>0 && FMath::FRand() < FMath::Clamp(DropChance,0.f,1.f))
            if (UAskirInventoryComponent* Inventory=Recipient->FindComponentByClass<UAskirInventoryComponent>())
                bItemGranted=Inventory->AddItem(GuaranteedDrop,GuaranteedDropQuantity);
        OnEnemyRewardsGranted(Recipient,XP,bItemGranted);
    }
    CancelPendingAttack();
    if (AAIController* AI = Cast<AAIController>(GetController())) AI->StopMovement();
    GetCharacterMovement()->DisableMovement();
    SetActorEnableCollision(false);
    SetLifeSpan(10.f);
}
