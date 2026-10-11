#include "TimerManager.h"
#include "Engine/World.h"
#include "AI/AskirBossAIController.h"
#include "Characters/AskirCharacter.h"
#include "Combat/AskirCombatComponent.h"

void AAskirBossAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    CancelTelegraphedAttack();
    BossPawn = Cast<AAskirCharacter>(InPawn);
    StaggerUntilTime = -1.f;
    if (IsValid(BossPawn.Get()) && IsValid(BossPawn->CombatComponent))
        BossPawn->CombatComponent->OnDamageTaken.AddDynamic(this, &AAskirBossAIController::HandleBossDamage);
    CurrentPhase = 1;
    LastTelegraphTime = -1000.f;
    InitialAttackInterval = AttackInterval;
    InitialAttackDistance = AttackDistance;
    UpdatePhase();
}

void AAskirBossAIController::Tick(float DeltaSeconds)
{
    if (IsValid(BossPawn.Get()) && BossPawn->IsDead()) CancelTelegraphedAttack();
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
    if (Now - LastTelegraphTime < FMath::Max(0.05f, AttackInterval)) return;
    LastTelegraphTime = Now;
    OnBossAttackTelegraph(CurrentPhase, FMath::Clamp(TelegraphWindupSeconds, 0.f, 5.f));
}

void AAskirBossAIController::OnUnPossess()
{
    CancelTelegraphedAttack();
    if (IsValid(BossPawn.Get()) && IsValid(BossPawn->CombatComponent))
        BossPawn->CombatComponent->OnDamageTaken.RemoveDynamic(this, &AAskirBossAIController::HandleBossDamage);
    BossPawn.Reset();
    StaggerUntilTime = -1.f;
    Super::OnUnPossess();
}

bool AAskirBossAIController::TryStartAttack(AAskirCharacter* Enemy, AAskirCharacter* Victim)
{
    if (bTelegraphPending || IsBossStaggered() || !IsValid(Enemy) || !IsValid(Victim) || Enemy->IsDead() || Victim->IsDead() || !GetWorld()) return false;
    const float Windup = FMath::Clamp(TelegraphWindupSeconds, 0.f, 5.f);
    if (Windup <= KINDA_SMALL_NUMBER) return Super::TryStartAttack(Enemy, Victim);
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
    if (!IsValid(Enemy) || !IsValid(Victim) || Enemy->IsDead() || Victim->IsDead() || GetPawn() != Enemy) return;
    // Recheck range and visibility at the end of the windup: no phantom hits after dodging.
    if (FVector::DistSquared(Enemy->GetActorLocation(), Victim->GetActorLocation()) > FMath::Square(FMath::Max(1.f, AttackDistance))) return;
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(AskirBossWindup), false, Enemy);
    const FVector Start = Enemy->GetActorLocation() + FVector(0.f, 0.f, 60.f);
    const FVector End = Victim->GetActorLocation() + FVector(0.f, 0.f, 60.f);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params) && Hit.GetActor() != Victim) return;
    Super::TryStartAttack(Enemy, Victim);
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
    if (!bAllowWindupInterrupt || !bTelegraphPending || !IsValid(BossPawn.Get()) || BossPawn->IsDead()) return;
    if (DamageInfo.Damage <= 0.f || DamageInfo.Damage < FMath::Max(0.f, StaggerDamageThreshold)) return;
    InterruptBossAttack();
}

void AAskirBossAIController::InterruptBossAttack()
{
    if (!bTelegraphPending || !GetWorld()) return;
    CancelTelegraphedAttack();
    const float Duration = FMath::Max(0.f, StaggerDurationSeconds);
    StaggerUntilTime = GetWorld()->GetTimeSeconds() + Duration;
    OnBossAttackInterrupted(CurrentPhase, Duration);
}
