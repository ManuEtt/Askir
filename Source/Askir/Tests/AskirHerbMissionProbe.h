#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AskirHerbMissionProbe.generated.h"

class UAskirHerbMissionComponent;
class AAskirGameMode;
class UAskirSaveGame;
class AAskirHerbPatchActor;
class AActor;

/** Actual dynamic-delegate receiver for herb-domain transaction tests. */
UCLASS()
class UAskirHerbMissionProbe : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() TObjectPtr<UAskirHerbMissionComponent> Mission;
    UPROPERTY() TObjectPtr<AAskirGameMode> Mode;
    UPROPERTY() TObjectPtr<UAskirSaveGame> Snapshot;
    UPROPERTY() TObjectPtr<AAskirHerbPatchActor> Patch;
    UPROPERTY() TObjectPtr<AActor> ExplorationActorToDestroy;
    bool bObservedAcceptance=false,bObservedCompletion=false,bGuardVisible=false;
    bool bReentrantAccept=false,bReentrantExplore=false,bReentrantReturn=false;
    bool bAcceptanceCaptureBlocked=false,bAcceptanceRestoreBlocked=false,bCompletionCaptureBlocked=false;
    bool bReceiptCommittedBeforeCallback=false;
    bool bObservedExploration=false,bExplorationReceiptCommitted=false,bExplorationDestroyAccepted=false;
    bool bPatchRebindAccepted=false,bPatchRefreshReturned=false;
    UFUNCTION() void ObserveReputationChange(FName Faction,int32 NewReputation);
    UFUNCTION() void ObserveQuestCompleted(FName QuestID);
    UFUNCTION() void ObserveExplorationChanged(FName QuestID,FName ObjectiveID);
};
