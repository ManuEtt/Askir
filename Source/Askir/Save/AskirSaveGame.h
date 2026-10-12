#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Simulation/AskirSimulationSubsystem.h"
#include "Quest/AskirQuestData.h"
#include "Progression/AskirProgressionComponent.h"
#include "Mission/AskirDornfeldMissionComponent.h"
#include "Simulation/AskirNeedsComponent.h"
#include "Social/AskirRelationshipComponent.h"
#include "AskirSaveGame.generated.h"
// Stable asset references survive across sessions without serializing transient UObject pointers.
USTRUCT(BlueprintType)
struct FAskirSavedInventoryEntry
{
    GENERATED_BODY()
    UPROPERTY(SaveGame) FSoftObjectPath ItemAsset;
    UPROPERTY(SaveGame) int32 Quantity = 0;
};

USTRUCT(BlueprintType)
struct FAskirLootState
{
 GENERATED_BODY()
 UPROPERTY(SaveGame) TArray<FName> CollectedPickups;
 UPROPERTY(SaveGame) TArray<FName> OpenedChests;
};

/** Explicit values, never actor/component pointers, for the existing player systems. */
USTRUCT()
struct FAskirPlayerSystemsSave
{
 GENERATED_BODY()
 UPROPERTY(SaveGame) float Warmth=50.f;
 UPROPERTY(SaveGame) float Wetness=0.f;
 UPROPERTY(SaveGame) float Exposure=0.f;
 UPROPERTY(SaveGame) float Noise=0.f;
 UPROPERTY(SaveGame) float Visibility=100.f;
 UPROPERTY(SaveGame) float Suspicion=0.f;
 UPROPERTY(SaveGame) float Morale=50.f;
 UPROPERTY(SaveGame) float Fear=0.f;
 UPROPERTY(SaveGame) float Courage=50.f;
 UPROPERTY(SaveGame) float TravelDistance=0.f;
 UPROPERTY(SaveGame) float TravelFatigue=0.f;
 UPROPERTY(SaveGame) float Supplies=100.f;
 UPROPERTY(SaveGame) float Fuel=0.f;
 UPROPERTY(SaveGame) float Food=0.f;
 UPROPERTY(SaveGame) float Shelter=0.f;
 UPROPERTY(SaveGame) float Loyalty=50.f;
 UPROPERTY(SaveGame) float Bond=0.f;
 UPROPERTY(SaveGame) float CompanionStress=0.f;
 UPROPERTY(SaveGame) int32 DiscoveredSites=0;
 UPROPERTY(SaveGame) int32 LoreKnowledge=0;
 UPROPERTY(SaveGame) float SurveyProgress=0.f;
 UPROPERTY(SaveGame) int32 Materials=0;
 UPROPERTY(SaveGame) int32 CraftedItems=0;
 UPROPERTY(SaveGame) int32 CraftSkill=1;
 UPROPERTY(SaveGame) TArray<FName> UnlockedRecipes;
 UPROPERTY(SaveGame) int32 MerchantFavor=0;
 UPROPERTY(SaveGame) int32 TradeVolume=0;
 UPROPERTY(SaveGame) float GuardStamina=100.f;
 UPROPERTY(SaveGame) FAskirNeeds Needs;
 UPROPERTY(SaveGame) FName Faction=NAME_None;
 UPROPERTY(SaveGame) TMap<FName,float> FactionStandings;
 UPROPERTY(SaveGame) TMap<FAskirEntityID,FAskirRelationship> Relationships;
};

UCLASS()
class ASKIR_API UAskirSaveGame:public USaveGame
{
    GENERATED_BODY()
public:
    /** Save format version; increment when persistent structures change. */
    static constexpr int32 CurrentFormatVersion = 3;
    UPROPERTY(SaveGame) int32 SaveFormatVersion = CurrentFormatVersion;
    /** Two alternating slot banks preserve the last valid generation on write failure. */
    UPROPERTY(SaveGame) int64 SaveGeneration = 0;
    /** Validates the complete persistent payload before gameplay changes begin. */
    bool ValidateSnapshot(FString& OutReason) const;
    static bool IsSafeSlotName(const FString& SlotName);
    /** V40: equipment snapshot; v1 saves retain defaults for compatibility. */
    UPROPERTY(SaveGame) FName SavedWeaponID = NAME_None;
    UPROPERTY(SaveGame) FName SavedArmorID = NAME_None;
    UPROPERTY(SaveGame) float SavedWeaponRating = 0.f;
    UPROPERTY(SaveGame) float SavedArmorRating = 0.f;
    UPROPERTY(SaveGame) float SavedGearDurability = 100.f;
    /** Level at the time of capture; helps diagnose cross-map restores. */
    UPROPERTY(SaveGame) FName SavedMapName = NAME_None;
    UPROPERTY(SaveGame) FString SavedMapPackageName;
    UPROPERTY(SaveGame) FAskirPlayerSystemsSave SavedSystems;
    // V15 persistent village/social snapshot. Runtime Save/Load mapping is still required.
    UPROPERTY(SaveGame) int32 SavedWorldDay=1;
    UPROPERTY(SaveGame) int32 SavedWorldHour=8;
    UPROPERTY(SaveGame) TMap<FName,int32> SavedFactionReputation;
    UPROPERTY(SaveGame) TArray<FName> SavedDialogueFlags;
    UPROPERTY(SaveGame) TArray<FName> CompletedOneShotWorldEvents;
    UPROPERTY(SaveGame) TMap<FName,int32> SavedWorldEventLastDays;
    UPROPERTY(SaveGame)int32 DemoStage=0;
    UPROPERTY(SaveGame)int32 DemoSupplies=0;
    UPROPERTY(SaveGame)bool bDemoFinished=false;
    UPROPERTY(SaveGame)bool bDemoEnemyDefeated=false;
    UPROPERTY(SaveGame)FAskirProgressionSnapshot SavedProgression;
    UPROPERTY(SaveGame)FAskirWorldState WorldState;
    UPROPERTY(SaveGame)FVector PlayerLocation=FVector::ZeroVector;
    UPROPERTY(SaveGame)FRotator PlayerRotation=FRotator::ZeroRotator;
    UPROPERTY(SaveGame)float PlayerHealth=100.f;
    UPROPERTY(SaveGame)float PlayerStamina=100.f;
    UPROPERTY(SaveGame)TArray<FAskirSavedInventoryEntry> SavedInventory;
    UPROPERTY(SaveGame)TMap<FName,EAskirQuestState> SavedQuestStates;
    UPROPERTY(SaveGame)TArray<FAskirQuestObjective> SavedActiveObjectives;
    UPROPERTY(SaveGame)TMap<FName,FAskirQuestObjectiveList> SavedObjectivesByQuest;
    UPROPERTY(SaveGame)FName SavedLegacyActiveQuestID=NAME_None;
    UPROPERTY(SaveGame)TSet<FName> SavedClaimedRewards;
    UPROPERTY(SaveGame)TSet<FName> SavedConsumedQuestTriggers;
    /** V66: persistent encounter completion ledger; absent in older saves. */
    UPROPERTY(SaveGame)TSet<FName> SavedCompletedEncounters;
    UPROPERTY(SaveGame)int32 SavedPlayerGold=0;
    UPROPERTY(SaveGame) FAskirDornfeldMissionSave SavedDornfeldMission;
    /** Map package -> persistent IDs for looted actors. */
    UPROPERTY(SaveGame)TMap<FName,FAskirLootState> SavedLootByMap;
};
