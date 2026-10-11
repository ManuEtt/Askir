#include "Characters/AskirNPC.h"
#include "NPC/AskirDailyRoutineComponent.h"
#include "NPC/AskirInteractionComponent.h"
#include "NPC/AskirDialogueComponent.h"
#include "AI/AskirCivilianAIController.h"
#include "AIController.h"
#include "Combat/AskirCombatComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AAskirNPC::AAskirNPC()
{
    PrimaryActorTick.bCanEverTick = true;
    CreateDefaultSubobject<UAskirDailyRoutineComponent>(TEXT("DailyRoutine"));
    CreateDefaultSubobject<UAskirInteractionComponent>(TEXT("Interaction"));
    CreateDefaultSubobject<UAskirDialogueComponent>(TEXT("Dialogue"));

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
    if (!bHostile || IsDead() || !GetWorld()) return;
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

    if (bVisible && bWithinLeash)
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
            }
        }
        return;
    }

    if (AIState == EAskirNPCState::Pursuing && bWithinLeash)
    {
        SearchEndTime = Now + SearchDuration;
        SetAIState(EAskirNPCState::Searching);
    }

    if (AIState == EAskirNPCState::Searching && Now < SearchEndTime && bWithinLeash)
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
    if (!Target || IsDead() || IsStaggered() || !GetWorld()) return;
    if (const AAskirCharacter* Character = Cast<AAskirCharacter>(Target))
    {
        if (Character->IsDead()) return;
    }
    // Revalidate at impact: dodging out of range or moving behind cover avoids damage.
    const float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (Distance > AttackDistance || !CanSeeTarget(Target))
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
        if (AAskirCharacter* Player = Cast<AAskirCharacter>(Target)) Player->ReceiveAskirDamage(Damage);
        else TargetCombat->ApplyDamage(Damage);
        OnAttackLanded();
    }
}

bool AAskirNPC::IsStaggered() const
{
    return GetWorld() && GetWorld()->GetTimeSeconds() < StaggerEndTime;
}

void AAskirNPC::HandleNPCDamage(const FAskirDamageInfo& DamageInfo)
{
    if(DamageInfo.DamageCauser && DamageInfo.Damage > 0.f) LastDamageCauser = DamageInfo.DamageCauser;
    if (!bHostile || !GetWorld() || IsDead() || DamageInfo.Damage < StaggerDamageThreshold) return;
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now < NextStaggerAllowedTime || StaggerDuration <= 0.f) return;
    CancelPendingAttack(); // A strong hit interrupts the telegraphed attack.
    StaggerEndTime = Now + StaggerDuration;
    NextStaggerAllowedTime = StaggerEndTime + StaggerImmunityDuration;
    NextAttackTime = FMath::Max(NextAttackTime, StaggerEndTime);
    if (AAIController* AI = Cast<AAIController>(GetController())) AI->StopMovement();
    OnStaggered(DamageInfo); // Blueprint animation or VFX hook, optional.
}

void AAskirNPC::HandleNPCDeath()
{
    if (!DefeatObjectiveID.IsNone() && LastDamageCauser.IsValid())
    {
        if (UAskirQuestComponent* Quests = LastDamageCauser->FindComponentByClass<UAskirQuestComponent>())
        {
            Quests->AdvanceObjective(DefeatObjectiveID, 1);
        }
    }
    CancelPendingAttack();
    if (AAIController* AI = Cast<AAIController>(GetController())) AI->StopMovement();
    GetCharacterMovement()->DisableMovement();
    SetActorEnableCollision(false);
    SetLifeSpan(10.f);
}
