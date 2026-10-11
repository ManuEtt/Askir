#pragma once
#include "CoreMinimal.h"
#include "AI/AskirEnemyAIController.h"
#include "AskirBossAIController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAskirBossPhaseTransition, int32, PreviousPhase, int32, NewPhase);

/** Health-driven boss phase transitions. Animation/VFX are optional Blueprint events. */
UCLASS(Blueprintable)
class ASKIR_API AAskirBossAIController : public AAskirEnemyAIController
{
    GENERATED_BODY()
public:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnUnPossess() override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss", meta=(ClampMin="0.01", ClampMax="0.99")) float PhaseTwoHealthFraction = 0.66f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss", meta=(ClampMin="0.01", ClampMax="0.99")) float PhaseThreeHealthFraction = 0.33f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss", meta=(ClampMin="0.1")) float PhaseTwoAttackSpeedMultiplier = 1.25f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss", meta=(ClampMin="0.1")) float PhaseThreeAttackSpeedMultiplier = 1.6f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss", meta=(ClampMin="0.1")) float PhaseTwoAttackRangeMultiplier = 1.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss", meta=(ClampMin="0.1")) float PhaseThreeAttackRangeMultiplier = 1.25f;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Askir|Boss") int32 CurrentPhase = 1;
    /** Broadcasts phase changes for HUD, audio and encounter scripting without requiring a BP override. */
    UPROPERTY(BlueprintAssignable, Category="Askir|Boss") FAskirBossPhaseTransition OnPhaseTransition;
    /** Triggered immediately before a boss attack when an attack becomes available. */
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Boss|Telegraph") void OnBossAttackTelegraph(int32 Phase, float WindupSeconds);
    /** Optional animation-only windup; combat still runs through the existing AI attack. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss|Telegraph", meta=(ClampMin="0.0", ClampMax="5.0")) float TelegraphWindupSeconds = 0.35f;
    UFUNCTION(BlueprintCallable, Category="Askir|Boss|Telegraph") void TriggerAttackTelegraph();
    /** Abort an uncommitted windup (e.g. on encounter reset). */
    UFUNCTION(BlueprintCallable, Category="Askir|Boss|Telegraph") void CancelTelegraphedAttack();
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Boss") void OnBossPhaseChanged(int32 OldPhase, int32 NewPhase);
    UFUNCTION(BlueprintPure, Category="Askir|Boss") int32 GetBossPhase() const { return CurrentPhase; }
protected:
    virtual bool TryStartAttack(AAskirCharacter* Enemy, AAskirCharacter* Victim) override;
private:
    UFUNCTION() void ResolveTelegraphedAttack();
    FTimerHandle TelegraphTimer;
    TWeakObjectPtr<AAskirCharacter> TelegraphTarget;
    bool bTelegraphPending = false;
    TWeakObjectPtr<class AAskirCharacter> BossPawn;
    float InitialAttackInterval = 1.4f;
    float InitialAttackDistance = 165.f;
    void UpdatePhase();
    float LastTelegraphTime = -1000.f;
};
