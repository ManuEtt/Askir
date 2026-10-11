#pragma once
#include "CoreMinimal.h"
#include "AI/AskirEnemyAIController.h"
#include "Combat/AskirDamageTypes.h"
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
    /** Damage during windup can stagger the boss; positive damage only. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss|Stagger", meta=(ClampMin="0.0")) float StaggerDamageThreshold = 20.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss|Stagger", meta=(ClampMin="0.0")) float StaggerDurationSeconds = 1.25f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss|Stagger") bool bAllowWindupInterrupt = true;
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Boss|Stagger") void OnBossAttackInterrupted(int32 Phase, float StaggerSeconds);
    UFUNCTION(BlueprintPure, Category="Askir|Boss|Stagger") bool IsBossStaggered() const;
    UFUNCTION(BlueprintCallable, Category="Askir|Boss|Stagger") void InterruptBossAttack();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss|Animation") TObjectPtr<class UAnimMontage> AttackMontage = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss|Animation") TObjectPtr<class UAnimMontage> StaggerMontage = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss|Animation", meta=(ClampMin="0.1")) float AttackMontagePlayRate = 1.f;
    /** Enable only when the attack montage has an AskirAttackImpact notify. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Boss|Animation") bool bImpactFromMontageNotify = false;
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Boss|Animation") void OnBossAttackCommitted(int32 Phase, bool bMontageStarted);
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Boss|Animation") void OnBossStaggerAnimation(int32 Phase, bool bMontageStarted);
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Boss") void OnBossPhaseChanged(int32 OldPhase, int32 NewPhase);
    UFUNCTION(BlueprintPure, Category="Askir|Boss") int32 GetBossPhase() const { return CurrentPhase; }
protected:
    virtual bool TryStartAttack(AAskirCharacter* Enemy, AAskirCharacter* Victim) override;
private:
    UFUNCTION() void ResolveTelegraphedAttack();
    UFUNCTION() void HandleBossDamage(const FAskirDamageInfo& DamageInfo);
    float StaggerUntilTime = -1.f;
    FTimerHandle TelegraphTimer;
    TWeakObjectPtr<AAskirCharacter> TelegraphTarget;
    bool bTelegraphPending = false;
    TWeakObjectPtr<class AAskirCharacter> BossPawn;
    float InitialAttackInterval = 1.4f;
    float InitialAttackDistance = 165.f;
    void UpdatePhase();
    bool CommitAnimatedAttack(AAskirCharacter* Enemy, AAskirCharacter* Victim);
    float LastTelegraphTime = -1000.f;
};
