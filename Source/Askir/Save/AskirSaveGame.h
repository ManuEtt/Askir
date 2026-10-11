#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Simulation/AskirSimulationSubsystem.h"
#include "Quest/AskirQuestData.h"
#include "Progression/AskirProgressionComponent.h"
#include "Mission/AskirDornfeldMissionComponent.h"
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

UCLASS()
class ASKIR_API UAskirSaveGame:public USaveGame
{
    GENERATED_BODY()
public:
    /** Save format version; increment when persistent structures change. */
    UPROPERTY(SaveGame) int32 SaveFormatVersion = 2;
    /** V40: equipment snapshot; v1 saves retain defaults for compatibility. */
    UPROPERTY(SaveGame) FName SavedWeaponID = NAME_None;
    UPROPERTY(SaveGame) FName SavedArmorID = NAME_None;
    UPROPERTY(SaveGame) float SavedWeaponRating = 0.f;
    UPROPERTY(SaveGame) float SavedArmorRating = 0.f;
    UPROPERTY(SaveGame) float SavedGearDurability = 100.f;
    /** Level at the time of capture; helps diagnose cross-map restores. */
    UPROPERTY(SaveGame) FName SavedMapName = NAME_None;
    // V15 persistent village/social snapshot. Runtime Save/Load mapping is still required.
    UPROPERTY(SaveGame) int32 SavedWorldDay=1;
    UPROPERTY(SaveGame) int32 SavedWorldHour=8;
    UPROPERTY(SaveGame) TMap<FName,int32> SavedFactionReputation;
    UPROPERTY(SaveGame) TArray<FName> SavedDialogueFlags;
    UPROPERTY(SaveGame) TArray<FName> CompletedOneShotWorldEvents;
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
    UPROPERTY(SaveGame)int32 SavedPlayerGold=0;
    UPROPERTY(SaveGame) FAskirDornfeldMissionSave SavedDornfeldMission;
    /** Map package -> persistent IDs for looted actors. */
    UPROPERTY(SaveGame)TMap<FName,FAskirLootState> SavedLootByMap;
};