#pragma once

#include "CoreMinimal.h"
#include "Characters/AskirCharacter.h"
#include "AskirNPC.generated.h"

class UAskirItemData;
UENUM(BlueprintType)
enum class EAskirNPCState : uint8
{
    Idle,
    Pursuing,
    Searching,
    Returning
};

class UAskirItemData;
UENUM(BlueprintType)
enum class EAskirEnemyStyle : uint8
{
    Balanced UMETA(DisplayName="Balanced"),
    Bruiser UMETA(DisplayName="Bruiser"),
    Skirmisher UMETA(DisplayName="Skirmisher")
};

UCLASS()
class ASKIR_API AAskirNPC : public AAskirCharacter
{
    GENERATED_BODY()
public:
    AAskirNPC();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|NPC") FText NPCName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Quest") FName DefeatObjectiveID;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Rewards", meta=(ClampMin="0")) int32 KillExperience=20;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Rewards") TObjectPtr<UAskirItemData> GuaranteedDrop=nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Rewards", meta=(ClampMin="0")) int32 GuaranteedDropQuantity=1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Rewards", meta=(ClampMin="0", ClampMax="1")) float DropChance=1.f;
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Rewards") void OnEnemyRewardsGranted(AActor* Recipient, int32 Experience, bool bItemGranted);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI") bool bHostile = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI", meta=(ClampMin="0")) float DetectionRadius = 1400.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI", meta=(ClampMin="0")) float LoseTargetRadius = 2000.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI", meta=(ClampMin="0")) float MaxChaseDistance = 2500.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI", meta=(ClampMin="0")) float SearchDuration = 4.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|AI", meta=(ClampMin="0.05")) float DecisionInterval = 0.2f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat", meta=(ClampMin="0")) float AttackDistance = 150.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat", meta=(ClampMin="0")) float AttackDamage = 10.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat", meta=(ClampMin="0.1")) float AttackInterval = 1.2f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat", meta=(ClampMin="0")) float AttackWindup = 0.35f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat", meta=(ClampMin="0")) float AttackRecovery = 0.25f;
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Combat") void OnAttackTelegraphed();
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Combat") void OnAttackLanded();
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Combat") void OnAttackAvoided();
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Combat") void OnAttackInterrupted();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat") EAskirEnemyStyle CombatStyle = EAskirEnemyStyle::Balanced;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat", meta=(ClampMin="0")) float BruiserDamageMultiplier = 1.6f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat", meta=(ClampMin="0.1")) float BruiserIntervalMultiplier = 1.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat", meta=(ClampMin="0.1")) float SkirmisherIntervalMultiplier = 0.7f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat", meta=(ClampMin="0.1")) float SkirmisherDamageMultiplier = 0.7f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat", meta=(ClampMin="0")) float SkirmisherRetreatDistance = 85.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat|Hit Reaction", meta=(ClampMin="0")) float StaggerDamageThreshold = 12.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat|Hit Reaction", meta=(ClampMin="0")) float StaggerDuration = 0.45f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat|Hit Reaction", meta=(ClampMin="0")) float StaggerImmunityDuration = 0.8f;
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Combat|Hit Reaction") void OnStaggered(const FAskirDamageInfo& DamageInfo);
    UFUNCTION(BlueprintPure, Category="Askir|Combat|Hit Reaction") bool IsStaggered() const;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Askir|AI") EAskirNPCState AIState = EAskirNPCState::Idle;

protected:
    UFUNCTION() void HandleNPCDeath();
    UFUNCTION() void HandleNPCDamage(const FAskirDamageInfo& DamageInfo);
    bool CanSeeTarget(const AActor* Target) const;
    void UpdateAI();
    void ConfigureCombatStyle();
    void SetAIState(EAskirNPCState NewState);
    void ResolvePendingAttack(float Now);
    void CancelPendingAttack();

private:
    FVector HomeLocation = FVector::ZeroVector;
    FVector LastKnownTargetLocation = FVector::ZeroVector;
    float NextDecisionTime = 0.f;
    float NextAttackTime = 0.f;
    float AttackResolveTime = -1.f;
    TWeakObjectPtr<AActor> PendingAttackTarget;
    float SearchEndTime = 0.f;
    float StaggerEndTime = -1.f;
    float NextStaggerAllowedTime = 0.f;
    TWeakObjectPtr<AActor> LastDamageCauser;
    bool bRewardsGranted=false;
};
