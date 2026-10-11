#include "TimerManager.h"
#include "Engine/World.h"
#include "AI/AskirBossAIController.h"
#include "Characters/AskirCharacter.h"
#include "Combat/AskirCombatComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

void AAskirBossAIController::OnPossess(APawn* InPawn)
{
    // A controller can be reused; remove callbacks from the previous pawn first.
    if (IsValid(BossPawn.Get()) && IsValid(BossPawn->CombatComponent))
        BossPawn->CombatComponent->OnDamageTaken.RemoveDynamic(this, &AAskirBossAIController::HandleBossDamage);
    CancelTelegraphedAttack();
    Super::OnPossess(InPawn);
    BossPawn = Cast<AAskirCharacter>(InPawn);
    StaggerUntilTime = -1.f;
    if (IsValid(BossPawn.Get()) && IsValid(BossPawn->CombatComponent))
        BossPawn->CombatComponent->OnDamageTaken.AddDynamic(this, &AAskirBossAIController::HandleBossDamage);
    // A controller can be reused after a phase-three fight. Restore its Blueprint
    // class defaults before capturing phase-one baselines for the new pawn.
    const AAskirBossAIController* Defaults = GetClass()->GetDefaultObject<AAskirBossAIController>();
    if (IsValid(Defaults))
    {
        AttackInterval = Defaults->AttackInterval;
        AttackDistance = Defaults->AttackDistance;
    }
    CurrentPhase = 1;
    LastTelegraphTime = -1000.f;
    InitialAttackInterval = AttackInterval;
    InitialAttackDistance = AttackDistance;
    UpdatePhase();
}

void AAskirBossAIController::Tick(float DeltaSeconds)
{
    if (IsValid(BossPawn.Get()) && BossPawn->IsDead())
    {
        CancelTelegraphedAttack();
        BossPawn->AbortPendingAttack();
        return; // Never schedule another attack from a dead boss.
    }
    // Apply transitions before the base AI chooses its next attack.
    UpdatePhase();
    Super::Tick(DeltaSeconds);
}

void AAskirBossAIController::UpdatePhase()
{
    AAskirCharacter* Pawn = BossPawn.Get();
    if (!IsValid(Pawn) || !IsValid(Pawn->CombatComponent) || Pawn->IsDead()) return;
    const float Fraction = Pawn->CombatComponent->GetHealthPercent();
    // Clamp threshold ordering: phase three must be reached after phase two.
    const float SecondThreshold = FMath::Clamp(PhaseTwoHealthFraction, 0.01f, 0.99f);
    const float ThirdThreshold = FMath::Clamp(PhaseThreeHealthFraction, 0.01f, SecondThreshold);
    const int32 DesiredPhase = Fraction <= ThirdThreshold ? 3 : (Fraction <= SecondThreshold ? 2 : 1);
    // Boss phases never regress after healing.
    if (DesiredPhase <= CurrentPhase) return;
    const int32 PreviousPhase = CurrentPhase;
    CurrentPhase = DesiredPhase;
    const float SpeedMultiplier = CurrentPhase == 3 ? PhaseThreeAttackSpeedMultiplier : PhaseTwoAttackSpeedMultiplier;
    const float RangeMultiplier = CurrentPhase == 3 ? PhaseThreeAttackRangeMultiplier : PhaseTwoAttackRangeMultiplier;
    AttackInterval = InitialAttackInterval / FMath::Max(0.1f, SpeedMultiplier);
    AttackDistance = InitialAttackDistance * FMath::Max(0.1f, RangeMultiplier);
    OnPhaseTransition.Broadcast(PreviousPhase, CurrentPhase);
    OnBossPhaseChanged(PreviousPhase, CurrentPhase);
}

void AAskirBossAIController::TriggerAttackTelegraph()
{
    if (!IsValid(BossPawn.Get()) || BossPawn->IsDead() || !GetWorld()) return;
    const float Now = GetWorld()->GetTimeSeconds();
    LastTelegraphTime = Now;
    OnBossAttackTelegraph(CurrentPhase, FMath::Clamp(TelegraphWindupSeconds, 0.f, 5.f));
}

void AAskirBossAIController::OnUnPossess()
{
    CancelTelegraphedAttack();
    if (IsValid(BossPawn.Get()))
    {
        BossPawn->AbortPendingAttack();
        BossPawn->SetImpactFromAnimationNotify(false);
        if (IsValid(BossPawn->CombatComponent))
            BossPawn->CombatComponent->OnDamageTaken.RemoveDynamic(this, &AAskirBossAIController::HandleBossDamage);
    }
    BossPawn.Reset();
    StaggerUntilTime = -1.f;
    Super::OnUnPossess();
}

bool AAskirBossAIController::TryStartAttack(AAskirCharacter* Enemy, AAskirCharacter* Victim)
{
    if (bTelegraphPending || IsBossStaggered() || !IsValid(Enemy) || !IsValid(Victim) || Enemy->IsDead() || Victim->IsDead() || !GetWorld()) return false;
    const float Windup = FMath::Clamp(TelegraphWindupSeconds, 0.f, 5.f);
    if (Windup <= KINDA_SMALL_NUMBER) return CommitAnimatedAttack(Enemy, Victim);
    TelegraphTarget = Victim;
    bTelegraphPending = true;
    TriggerAttackTelegraph();
    GetWorldTimerManager().SetTimer(TelegraphTimer, this, &AAskirBossAIController::ResolveTelegraphedAttack, Windup, false);
    return true;
}

void AAskirBossAIController::ResolveTelegraphedAttack()
{
    if (!bTelegraphPending) return;
    if (IsBossStaggered()) { CancelTelegraphedAttack(); return; }
    bTelegraphPending = false;
    AAskirCharacter* Enemy = BossPawn.Get();
    AAskirCharacter* Victim = TelegraphTarget.Get();
    TelegraphTarget.Reset();
    if (!GetWorld() || !IsValid(Enemy) || !IsValid(Victim) || Enemy->IsDead() || Victim->IsDead() || GetPawn() != Enemy) return;
    // Recheck range and visibility at the end of the windup: no phantom hits after dodging.
    if (FVector::DistSquared(Enemy->GetActorLocation(), Victim->GetActorLocation()) > FMath::Square(FMath::Max(1.f, AttackDistance))) return;
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(AskirBossWindup), false, Enemy);
    const FVector Start = Enemy->GetActorLocation() + FVector(0.f, 0.f, 60.f);
    const FVector End = Victim->GetActorLocation() + FVector(0.f, 0.f, 60.f);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params) && Hit.GetActor() != Victim) return;
    // The windup can expire while stamina or attack cooldown still blocks the pawn.
    // In that case the boss returns to its normal decision loop without playing a montage.
    CommitAnimatedAttack(Enemy, Victim);
}

bool AAskirBossAIController::CommitAnimatedAttack(AAskirCharacter* Enemy, AAskirCharacter* Victim)
{
    if (!IsValid(Enemy) || !IsValid(Victim) || Enemy->IsDead() || Victim->IsDead()) return false;
    UAnimInstance* Anim = Enemy->GetMesh() ? Enemy->GetMesh()->GetAnimInstance() : nullptr;
    const bool bCanPlay = IsValid(AttackMontage) && IsValid(Anim);
    Enemy->SetImpactFromAnimationNotify(bCanPlay && bImpactFromMontageNotify);
    // The base AI reports that Attack() was invoked, not whether stamina/cooldown
    // actually allowed an attack. Require a pending hit before playing a montage.
    const bool bRequested = Super::TryStartAttack(Enemy, Victim);
    const bool bCommitted = bRequested && Enemy->HasPendingAttack();
    if (!bCommitted) Enemy->SetImpactFromAnimationNotify(false);
    bool bStarted = false;
    if (bCommitted && bCanPlay)
        bStarted = Anim->Montage_Play(AttackMontage, FMath::Max(0.1f, AttackMontagePlayRate)) > 0.f;
    if (bCommitted && !bStarted && bImpactFromMontageNotify)
        Enemy->FallbackPendingAttackToTimer();
    if (bCommitted) OnBossAttackCommitted(CurrentPhase, bStarted);
    return bCommitted;
}

void AAskirBossAIController::CancelTelegraphedAttack()
{
    if (GetWorld()) GetWorldTimerManager().ClearTimer(TelegraphTimer);
    bTelegraphPending = false;
    TelegraphTarget.Reset();
}

bool AAskirBossAIController::IsBossStaggered() const
{
    return GetWorld() && GetWorld()->GetTimeSeconds() < StaggerUntilTime;
}

void AAskirBossAIController::HandleBossDamage(const FAskirDamageInfo& DamageInfo)
{
    if (!IsValid(BossPawn.Get()) || BossPawn->IsDead()) return;
    const bool bWindup = bTelegraphPending && bAllowWindupInterrupt;
    const bool bCommitted = bAllowCommittedAttackInterrupt && BossPawn->HasPendingAttack();
    if (!bWindup && !bCommitted) return;
    if (DamageInfo.Damage <= 0.f || DamageInfo.Damage < FMath::Max(0.f, StaggerDamageThreshold)) return;
    InterruptBossAttack();
}

void AAskirBossAIController::InterruptBossAttack()
{
    if (!GetWorld() || !IsValid(BossPawn.Get())) return;
    if (!bTelegraphPending && !BossPawn->HasPendingAttack()) return;
    CancelTelegraphedAttack();
    if (IsValid(BossPawn.Get()))
    {
        BossPawn->AbortPendingAttack();
        if (UAnimInstance* Anim = BossPawn->GetMesh() ? BossPawn->GetMesh()->GetAnimInstance() : nullptr)
        {
            if (IsValid(AttackMontage)) Anim->Montage_Stop(0.12f, AttackMontage);
            const bool bStarted = IsValid(StaggerMontage) && Anim->Montage_Play(StaggerMontage) > 0.f;
            OnBossStaggerAnimation(CurrentPhase, bStarted);
        }
        else OnBossStaggerAnimation(CurrentPhase, false);
    }
    const float Duration = FMath::Max(0.f, StaggerDurationSeconds);
    StaggerUntilTime = GetWorld()->GetTimeSeconds() + Duration;
    OnBossAttackInterrupted(CurrentPhase, Duration);
}
