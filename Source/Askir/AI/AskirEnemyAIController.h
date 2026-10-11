#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "AskirEnemyAIController.generated.h"

class AAskirCharacter;

UENUM(BlueprintType)
enum class EAskirEnemyState : uint8
{
    Patrol, Investigate, Chase, Attack, Retreat, ReturnHome, Dead
};

/** Lightweight single-player enemy AI; requires a NavMeshBoundsVolume. */
UCLASS(Blueprintable)
class ASKIR_API AAskirEnemyAIController : public AAIController
{
    GENERATED_BODY()
public:
    AAskirEnemyAIController();
    virtual void OnPossess(APawn* InPawn) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI|Senses") float SightRange = 1800.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI|Senses") float FieldOfViewDegrees = 110.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI|Senses") float HearingRange = 1200.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI|Senses") float MemorySeconds = 7.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI|Movement") float PatrolRadius = 700.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI|Movement") float MaxChaseFromHome = 2800.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI|Movement") float AttackDistance = 165.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI|Movement") float RetreatHealthFraction = .2f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI|Movement") float DecisionInterval = .3f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI|Combat") float AttackInterval = 1.4f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI|Combat") bool bHostileToPlayer = true;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Askir|AI") EAskirEnemyState State = EAskirEnemyState::Patrol;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Askir|AI") FVector HomeLocation = FVector::ZeroVector;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Askir|AI") FVector LastKnownTargetLocation = FVector::ZeroVector;

    /** Explicit hearing stimulus: call from footstep/weapon noise Blueprints. */
    UFUNCTION(BlueprintCallable, Category="Askir|AI|Senses") void HearNoise(AActor* InstigatorActor, FVector NoiseLocation, float Loudness = 1.f);
    UFUNCTION(BlueprintCallable, Category="Askir|AI") void SetPatrolPoints(const TArray<FVector>& WorldPoints);
    UFUNCTION(BlueprintCallable, Category="Askir|AI") void ForceInvestigate(FVector WorldLocation);
    UFUNCTION(BlueprintPure, Category="Askir|AI") FString GetStateLabel() const;
private:
    UPROPERTY(Transient) TObjectPtr<AAskirCharacter> ControlledEnemy;
    UPROPERTY(Transient) TObjectPtr<AAskirCharacter> Target;
    TArray<FVector> PatrolPoints;
    int32 PatrolIndex = 0;
    float DecisionElapsed = 0.f;
    float LastSeenTime = -10000.f;
    float LastAttackTime = -10000.f;
    bool CanSeeTarget(const AAskirCharacter* Candidate) const;
    void Think();
    void MoveToPoint(const FVector& Destination, float AcceptanceRadius = 90.f);
    void SetState(EAskirEnemyState NewState);
};
