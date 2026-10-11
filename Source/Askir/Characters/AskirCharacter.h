#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Combat/AskirDamageTypes.h"
#include "TimerManager.h"
#include "AskirCharacter.generated.h"
class UAskirDornfeldMissionComponent; class UAskirGuardComponent; class USpringArmComponent; class UCameraComponent; class UAskirCombatComponent; class UAskirInventoryComponent; class UAskirQuestComponent; class UAskirProgressionComponent;
class UAskirSurvivalComponent;
class UAskirEquipmentComponent;
class UAskirStealthComponent;
class UAskirMoraleComponent;
class UAskirTravelComponent;
class UAskirTradeComponent;
class UAskirCampComponent;
class UAskirCompanionComponent;
class UAskirExplorationComponent;
class UAskirCraftingComponent;
class UAskirNeedsComponent;
class UAskirAnimationStateComponent;
UCLASS()
class ASKIR_API AAskirCharacter:public ACharacter
{
    GENERATED_BODY()
public:
    AAskirCharacter();
    virtual void BeginPlay()override;
    virtual void Tick(float DeltaSeconds)override;
    virtual void SetupPlayerInputComponent(UInputComponent*)override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UAskirCombatComponent> CombatComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Askir|Combat") TObjectPtr<UAskirGuardComponent> GuardComponent;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UAskirInventoryComponent> InventoryComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Askir|Quest") TObjectPtr<UAskirQuestComponent> QuestComponent;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Progression") TObjectPtr<UAskirProgressionComponent> ProgressionComponent;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Systems") TObjectPtr<UAskirSurvivalComponent> SurvivalComponent;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Systems") TObjectPtr<UAskirEquipmentComponent> EquipmentComponent;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Systems") TObjectPtr<UAskirStealthComponent> StealthComponent;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Systems") TObjectPtr<UAskirMoraleComponent> MoraleComponent;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Systems") TObjectPtr<UAskirTravelComponent> TravelComponent;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Systems") TObjectPtr<UAskirTradeComponent> TradeComponent;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Systems") TObjectPtr<UAskirCampComponent> CampComponent;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Systems") TObjectPtr<UAskirCompanionComponent> CompanionComponent;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Systems") TObjectPtr<UAskirExplorationComponent> ExplorationComponent;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Systems") TObjectPtr<UAskirCraftingComponent> CraftingComponent;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Systems") TObjectPtr<UAskirNeedsComponent> NeedsComponent;
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Survival") bool EatFromCamp(float Amount);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Survival") bool DrinkAndRecover(float Amount);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Survival") bool WarmAtCamp(float Hours);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Survival") bool ColdTravel(float Hours);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Survival") bool DryInShelter(float Amount);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Equipment") bool DamageEquipmentOnHit(float Damage);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Equipment") bool RepairUsingMaterials(int32 MaterialsCost);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Equipment") bool EquipArmorForProtection(float Rating);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Equipment") bool EquipWeaponForDamage(float Rating);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Equipment") bool SellDamagedGear(int32 GoldValue);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Stealth") bool CrouchAndQuiet();
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Stealth") bool RaiseAlarm(float Severity);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Stealth") bool SneakTravel(float Distance);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Stealth") bool HideAtCamp();
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Stealth") bool RecoverAfterAlert(float Minutes);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Morale") bool EncourageCompanion(float Amount);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Morale") bool FearFromAmbush(float Amount);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Morale") bool RestMorale(float Hours);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Morale") bool TrainWithCompanion(float Hours);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Morale") bool LoseMoraleFromHunger(float Hunger);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Travel") bool TravelWithSupplies(float Distance);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Travel") bool BuyTravelSupplies(int32 GoldCost);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Travel") bool RestAtCamp(float Hours);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Travel") bool FastTravelWithCost(float Distance);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Travel") bool SurveyWhileTraveling(float Distance);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Trade") bool SellCraftedItem(int32 Price);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Trade") bool BuyMaterials(int32 Count);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Trade") bool BuyCampFuel(int32 Count);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Trade") bool BuyCampFood(int32 Count);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Trade") bool RewardExploration(int32 GoldReward);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Camp") bool BuildShelterFromMaterials(int32 Count);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Camp") bool CollectWoodWhileTraveling(float Distance);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Camp") bool CookRations(float Amount);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Camp") bool ShelterCompanion(float Hours);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Camp") bool BreakCamp();
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Companion") bool RewardCompanion(int32 Gold);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Companion") bool TrainCompanion(float Hours);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Companion") bool ComfortCompanionAtFire(float Hours);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Companion") bool TravelTogether(float Distance);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Companion") bool CompanionUnderThreat(float Threat);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Exploration") bool DiscoverAndReward(int32 Experience);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Exploration") bool StudyAndLearn(int32 Hours);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Exploration") bool SurveyWithSupplies(float Distance);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Exploration") bool GatherAtDiscovery(int32 Amount);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Exploration") bool ShareLoreWithCompanion(int32 Lore);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Crafting") bool CraftAndEarnExperience(int32 Cost);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Crafting") bool SalvageForCamp(int32 Amount);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Crafting") bool TrainCraftingAtCamp();
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Crafting") bool CraftRepairKit(int32 Cost);
    UFUNCTION(BlueprintCallable,Category="Askir|Integrated|Crafting") bool TradeCraftForSupplies(int32 SupplyAmount);
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Dornfeld") TObjectPtr<UAskirDornfeldMissionComponent> DornfeldMission;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Animation") TObjectPtr<UAskirAnimationStateComponent> AnimationState;
    UPROPERTY(EditAnywhere)float WalkSpeed=450;
    UPROPERTY(EditAnywhere)float SprintSpeed=700;
    UPROPERTY(EditAnywhere)float BaseAttackDamage=15;
    UPROPERTY(EditAnywhere)float AttackRange=150;
    UPROPERTY(EditAnywhere)float AttackRadius=35;
    UPROPERTY(EditAnywhere)float AttackCooldown=.65;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Stamina") float MaxStamina=100.f;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Askir|Stamina") float Stamina=100.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Stamina") float StaminaRegenPerSecond=18.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Stamina") float StaminaRegenDelay=1.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat") float LightAttackCost=15.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat") float HeavyAttackCost=32.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat") float HeavyDamageMultiplier=1.8f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat") float HeavyAttackCooldown=1.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat") float DodgeCost=25.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat") float DodgeStrength=650.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat") float DodgeCooldown=0.85f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat", meta=(ClampMin="0.0", ClampMax="0.5")) float DodgeInvulnerabilitySeconds=0.22f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat") float BlockDamageReduction=0.6f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat") float BlockStaminaPerDamage=0.5f;
    UFUNCTION(BlueprintPure, Category="Askir|Stamina") float GetStaminaPercent()const{return MaxStamina>0.f?Stamina/MaxStamina:0.f;}
    // Optional Blueprint animation/audio hooks. Combat works without assigned assets.
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Combat|Feedback") void OnAttackStarted(bool bHeavyAttack);
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Combat|Feedback") void OnAttackHit(AActor* HitActor, FVector HitLocation, bool bHeavyAttack);
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Combat|Feedback") void OnAttackMissed(bool bHeavyAttack);
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Combat|Feedback") void OnDodgeStarted();
    UFUNCTION(BlueprintImplementableEvent, Category="Askir|Combat|Feedback") void OnBlockStateChanged(bool bIsBlocking);
    UFUNCTION(BlueprintCallable)void Attack();
    UFUNCTION(BlueprintCallable)void HeavyAttack();
    UFUNCTION(BlueprintCallable)void Dodge();
    UFUNCTION(BlueprintCallable)void StartBlock();
    UFUNCTION(BlueprintCallable)void StopBlock();
    UFUNCTION(BlueprintCallable)void Heal(float Amount);
    UFUNCTION(BlueprintCallable)void ReceiveAskirDamage(const FAskirDamageInfo&);
    UFUNCTION(BlueprintPure)bool IsDead()const;
protected:
    void MoveForward(float);void MoveRight(float);void Turn(float);void LookUp(float);
    void StartSprint();void StopSprint();
    void DemoInteractInput();void DemoSaveInput();void DemoLoadInput();
    UFUNCTION()void HandleDeath();
    void PerformAttack(float DamageMultiplier, float Cost, float Cooldown, bool bHeavyAttack);
    bool SpendStamina(float Cost);
private:
    // V6: a single damage window per committed attack, opened by an AnimNotify or timer fallback.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat|Timing") float LightImpactDelay=0.22f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat|Timing") float HeavyImpactDelay=0.48f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Combat|Timing") bool bUseAnimationNotifyForImpact=false;
    UFUNCTION(BlueprintCallable, Category="Askir|Combat|Timing") void ResolvePendingAttackImpact();
    UFUNCTION(BlueprintPure, Category="Askir|Combat|Timing") bool HasPendingAttack() const {return bPendingAttack;}
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Askir|Combat|Timing") bool bPendingAttack=false;
    float PendingDamageMultiplier=1.f;
    bool bPendingHeavyAttack=false;
    FTimerHandle PendingImpactTimer;
    void CancelPendingAttack();
    float LastAttackTime=-1000.f;
    float LastDodgeTime=-1000.f;
    float LastStaminaUseTime=-1000.f;
    bool bBlocking=false;
};