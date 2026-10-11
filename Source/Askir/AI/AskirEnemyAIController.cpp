#include "AI/AskirEnemyAIController.h"
#include "Characters/AskirCharacter.h"
#include "Combat/AskirCombatComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AAskirEnemyAIController::AAskirEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}
void AAskirEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    ControlledEnemy = Cast<AAskirCharacter>(InPawn);
    if (ControlledEnemy) HomeLocation = ControlledEnemy->GetActorLocation();
    State = EAskirEnemyState::Patrol;
    DecisionElapsed = DecisionInterval;
}
void AAskirEnemyAIController::SetState(EAskirEnemyState NewState)
{
    if (State == NewState) return;
    State = NewState;
    if (State == EAskirEnemyState::Dead || State == EAskirEnemyState::Attack) StopMovement();
}
void AAskirEnemyAIController::MoveToPoint(const FVector& Destination, float AcceptanceRadius)
{
    if (GetPawn()) MoveToLocation(Destination, AcceptanceRadius, true, true, false, true);
}
void AAskirEnemyAIController::SetPatrolPoints(const TArray<FVector>& WorldPoints)
{
    PatrolPoints = WorldPoints;
    PatrolIndex = 0;
}
void AAskirEnemyAIController::ForceInvestigate(FVector WorldLocation)
{
    if (!ControlledEnemy || ControlledEnemy->IsDead()) return;
    LastKnownTargetLocation = WorldLocation;
    LastSeenTime = GetWorld()->GetTimeSeconds();
    SetState(EAskirEnemyState::Investigate);
}
void AAskirEnemyAIController::HearNoise(AActor* InstigatorActor, FVector NoiseLocation, float Loudness)
{
    if (!ControlledEnemy || !InstigatorActor || !bHostileToPlayer || ControlledEnemy->IsDead()) return;
    const float EffectiveRange = HearingRange * FMath::Clamp(Loudness, 0.f, 3.f);
    if (FVector::DistSquared(ControlledEnemy->GetActorLocation(), NoiseLocation) > FMath::Square(EffectiveRange)) return;
    AAskirCharacter* Candidate = Cast<AAskirCharacter>(InstigatorActor);
    if (!Candidate || Candidate == ControlledEnemy || Candidate->IsDead() || !Candidate->IsPlayerControlled()) return;
    Target = Candidate;
    ForceInvestigate(NoiseLocation);
}
bool AAskirEnemyAIController::CanSeeTarget(const AAskirCharacter* Candidate) const
{
    if (!ControlledEnemy || !Candidate || Candidate->IsDead()) return false;
    const FVector Origin = ControlledEnemy->GetActorLocation() + FVector(0,0,60);
    const FVector End = Candidate->GetActorLocation() + FVector(0,0,60);
    const FVector Delta = End - Origin;
    if (Delta.SizeSquared() > FMath::Square(SightRange)) return false;
    const FVector Forward = ControlledEnemy->GetActorForwardVector();
    if (FVector::DotProduct(Forward, Delta.GetSafeNormal()) < FMath::Cos(FMath::DegreesToRadians(FieldOfViewDegrees * .5f))) return false;
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(AskirAISight), false, ControlledEnemy);
    if (!GetWorld()->LineTraceSingleByChannel(Hit, Origin, End, ECC_Visibility, Params)) return true;
    return Hit.GetActor() == Candidate;
}
void AAskirEnemyAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!ControlledEnemy) return;
    DecisionElapsed += DeltaSeconds;
    if (DecisionElapsed >= FMath::Max(.05f, DecisionInterval))
    {
        DecisionElapsed = 0.f;
        Think();
    }
}
void AAskirEnemyAIController::Think()
{
    if (!ControlledEnemy || !GetWorld()) return;
    if (ControlledEnemy->IsDead()) { Target = nullptr; SetState(EAskirEnemyState::Dead); return; }
    const float Now = GetWorld()->GetTimeSeconds();
    const FVector MyPos = ControlledEnemy->GetActorLocation();
    if (FVector::DistSquared(MyPos, HomeLocation) > FMath::Square(MaxChaseFromHome))
    {
        Target = nullptr;
        SetState(EAskirEnemyState::ReturnHome);
    }
    if (State == EAskirEnemyState::ReturnHome)
    {
        if (FVector::DistSquared(MyPos, HomeLocation) < FMath::Square(110.f)) SetState(EAskirEnemyState::Patrol);
        else MoveToPoint(HomeLocation);
        return;
    }
    if (ControlledEnemy->CombatComponent && ControlledEnemy->CombatComponent->GetHealthPercent() <= RetreatHealthFraction && IsValid(Target.Get()))
    {
        SetState(EAskirEnemyState::Retreat);
        const FVector Away = (MyPos - Target->GetActorLocation()).GetSafeNormal();
        MoveToPoint(MyPos + Away * 600.f);
        return;
    }
    AAskirCharacter* Player = Cast<AAskirCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (bHostileToPlayer && Player && Player != ControlledEnemy && CanSeeTarget(Player))
    {
        Target = Player;
        LastSeenTime = Now;
        LastKnownTargetLocation = Player->GetActorLocation();
    }
    if (Target && (Target->IsDead() || !bHostileToPlayer)) Target = nullptr;
    const bool bFreshMemory = Target && Now - LastSeenTime <= MemorySeconds;
    if (bFreshMemory)
    {
        const bool bVisible = CanSeeTarget(Target);
        const float Dist = FVector::Dist(MyPos, Target->GetActorLocation());
        if (bVisible && Dist <= AttackDistance)
        {
            SetState(EAskirEnemyState::Attack);
            SetFocus(Target);
            if (Now - LastAttackTime >= AttackInterval && !ControlledEnemy->HasPendingAttack())
            {
                LastAttackTime = Now;
                ControlledEnemy->Attack();
            }
            return;
        }
        SetState(bVisible ? EAskirEnemyState::Chase : EAskirEnemyState::Investigate);
        MoveToPoint(LastKnownTargetLocation, 80.f);
        return;
    }
    Target = nullptr;
    ClearFocus(EAIFocusPriority::Gameplay);
    if (State == EAskirEnemyState::Investigate || State == EAskirEnemyState::Chase || State == EAskirEnemyState::Attack)
    {
        SetState(EAskirEnemyState::ReturnHome);
        MoveToPoint(HomeLocation);
        return;
    }
    SetState(EAskirEnemyState::Patrol);
    if (PatrolPoints.Num())
    {
        if (FVector::DistSquared(MyPos, PatrolPoints[PatrolIndex]) < FMath::Square(120.f)) PatrolIndex = (PatrolIndex + 1) % PatrolPoints.Num();
        MoveToPoint(PatrolPoints[PatrolIndex]);
    }
    else
    {
        // Without patrol points, stay near the spawn instead of issuing a new random move every tick.
        if (FVector::DistSquared(MyPos, HomeLocation) > FMath::Square(200.f)) MoveToPoint(HomeLocation);
        else StopMovement();
    }
}
FString AAskirEnemyAIController::GetStateLabel() const
{
    const UEnum* Enum = StaticEnum<EAskirEnemyState>();
    return Enum ? Enum->GetNameStringByValue(static_cast<int64>(State)) : TEXT("Unknown");
}
