#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Simulation/AskirSimulationSubsystem.h"
#include "Quest/AskirQuestData.h"
#include "Progression/AskirProgressionComponent.h"
#include "AskirSaveGame.generated.h"
// Stable asset references survive across sessions without serializing transient UObject pointers.
USTRUCT(BlueprintType)
struct FAskirSavedInventoryEntry
{
    GENERATED_BODY()
    UPROPERTY(SaveGame) FSoftObjectPath ItemAsset;
    UPROPERTY(SaveGame) int32 Quantity = 0;
};

UCLASS()
class ASKIR_API UAskirSaveGame:public USaveGame
{
    GENERATED_BODY()
public:
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
};