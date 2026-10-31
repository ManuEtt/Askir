#include "Tests/AskirHerbMissionProbe.h"
#include "Mission/AskirHerbMissionComponent.h"
#include "Mission/AskirHerbPatchActor.h"
#include "Game/AskirGameMode.h"
#include "Quest/AskirQuestComponent.h"
#include "Save/AskirSaveGame.h"
#include "GameFramework/Actor.h"

void UAskirHerbMissionProbe::ObserveReputationChange(FName Faction,int32 NewReputation)
{
    if(!Mission)return;bObservedAcceptance=true;bGuardVisible=Mission->IsTransactionInProgress();
    const auto* Journal=Mission->GetOwner()?Mission->GetOwner()->FindComponentByClass<UAskirQuestComponent>():nullptr;
    bReceiptCommittedBeforeCallback=Journal&&Journal->HasConsumedQuestTrigger(FName(TEXT("SQ_003_ACCEPTED")));
    bReentrantAccept=Mission->AcceptHerbs(FName(TEXT("bargain")));
    bReentrantExplore=Mission->ExploreHerbs(UAskirHerbMissionComponent::GetCanonicalSourceID());
    bAcceptanceCaptureBlocked=Mode&&Mode->CaptureSnapshot()==nullptr;
    bAcceptanceRestoreBlocked=Mode&&Snapshot&&!Mode->RestoreSnapshot(Snapshot,false);
}
void UAskirHerbMissionProbe::ObserveQuestCompleted(FName QuestID)
{
    if(!Mission||QuestID!=UAskirHerbMissionComponent::GetQuestID())return;
    bObservedCompletion=true;bReentrantReturn=Mission->ReturnToRurik();
    bCompletionCaptureBlocked=Mode&&Mode->CaptureSnapshot()==nullptr;
}
void UAskirHerbMissionProbe::ObserveExplorationChanged(FName QuestID,FName ObjectiveID)
{
    if(!Mission||QuestID!=UAskirHerbMissionComponent::GetQuestID()||ObjectiveID!=FName(TEXT("SQ_003_A")))return;
    bObservedExploration=true;
    bExplorationReceiptCommitted=Mission->HasExploredHerbs(UAskirHerbMissionComponent::GetCanonicalSourceID());
    if(IsValid(ExplorationActorToDestroy))bExplorationDestroyAccepted=ExplorationActorToDestroy->Destroy();
    if(Patch)
    {
        bPatchRebindAccepted=Patch->BindMissionEvents();
        Patch->RefreshMissionState();bPatchRefreshReturned=true;
    }
}
