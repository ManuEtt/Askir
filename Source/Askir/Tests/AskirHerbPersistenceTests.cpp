#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Tests/AskirTransactionPersistenceProbe.h"
#include "Characters/AskirCharacter.h"
#include "Game/AskirGameMode.h"
#include "Mission/AskirHerbMissionComponent.h"
#include "Mission/AskirHerbPatchActor.h"
#include "NPC/AskirDialogueComponent.h"
#include "Social/AskirReputationComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Quest/AskirQuestData.h"
#include "Save/AskirSaveGame.h"
#include "Trade/AskirTradeComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Progression/AskirProgressionComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
struct FHerbPersistenceFixture : FAskirAutomationTestWorld
{
    AAskirCharacter* Player=nullptr;
    APlayerController* Controller=nullptr;
    AAskirGameMode* Mode=nullptr;
    FHerbPersistenceFixture()
    {
        if(!World)return;
        Player=Spawn<AAskirCharacter>(FVector(0.,0.,96.));Controller=Spawn<APlayerController>();Mode=Spawn<AAskirGameMode>();
        if(Player&&Controller)
        {
            Controller->Possess(Player);Player->GetCharacterMovement()->DisableMovement();
            Player->TradeComponent->Gold=0;Player->InventoryComponent->Items.Reset();
            Player->ProgressionComponent->RestoreProgress(FAskirProgressionSnapshot());
        }
    }
    bool Ready() const {return World&&Player&&Controller&&Mode&&Player->HerbMission;}
    UAskirItemData* PrepareReward()
    {
        UPackage* Package=CreatePackage(*(FString(TEXT("/Game/Askir/Automation/HerbPersistence_"))+FGuid::NewGuid().ToString(EGuidFormats::Digits)));
        auto* Item=NewObject<UAskirItemData>(Package,TEXT("item_004"),RF_Public);
        Item->ItemID=TEXT("item_004");Item->MaxStackSize=99;
        auto* Data=Player->HerbMission->ResolveQuestData();
        if(Data)Data->RewardItem=Item;
        return Data?Item:nullptr;
    }
};

UAskirSaveGame* HerbSaveFixture()
{
    auto* Save=NewObject<UAskirSaveGame>();
    Save->SavedQuestStates.Add(TEXT("SQ_003"),EAskirQuestState::Active);
    Save->SavedConsumedQuestTriggers.Add(TEXT("SQ_003_ACCEPTED"));
    Save->SavedConsumedQuestTriggers.Add(TEXT("SQ_003_EXPLORED"));
    Save->SavedConsumedQuestTriggers.Add(TEXT("SQ_003_EXPLORED_SOURCE_SQ003_Meadow_V84"));
    Save->SavedDialogueFlags.Add(TEXT("SQ_003_CHOICE_help"));
    FAskirQuestObjective Explore;Explore.ObjectiveID=TEXT("SQ_003_A");Explore.RequiredCount=1;Explore.CurrentCount=1;
    FAskirQuestObjective Return;Return.ObjectiveID=TEXT("SQ_003_B");Return.RequiredCount=1;Return.CurrentCount=0;
    Save->SavedObjectivesByQuest.FindOrAdd(TEXT("SQ_003")).Objectives={Explore,Return};
    Save->SavedLegacyActiveQuestID=TEXT("SQ_003");Save->SavedActiveObjectives={Explore,Return};
    return Save;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirHerbSaveRoundTripTest,"Askir.Persistence.HerbExploreReceiptRestoresVisibleTimeline",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirHerbSaveRoundTripTest::RunTest(const FString& Parameters)
{
    FHerbPersistenceFixture Fixture;if(!TestTrue(TEXT("Native player owns herb mission and save mode"),Fixture.Ready()))return false;
    auto* Patch=Fixture.Spawn<AAskirHerbPatchActor>(FVector(160.,0.,100.));
    if(!TestNotNull(TEXT("Real native exploration actor"),Patch))return false;
    Patch->SourceID=TEXT("SQ003_Meadow_V84");
    auto* Probe=NewObject<UAskirTransactionPersistenceProbe>();Probe->Mode=Fixture.Mode;Probe->Snapshot=Fixture.Mode->CaptureSnapshot();
    if(!TestNotNull(TEXT("Snapshot before acceptance"),Probe->Snapshot.Get()))return false;
    auto* Reputation=Fixture.Player->FindComponentByClass<UAskirReputationComponent>();
    if(!TestNotNull(TEXT("Saved reputation ledger"),Reputation))return false;
    Reputation->OnReputationChanged.AddDynamic(Probe,&UAskirTransactionPersistenceProbe::ObserveReputationChange);
    const bool bAccepted=Fixture.Player->HerbMission->AcceptHerbs(TEXT("bargain"));
    Reputation->OnReputationChanged.RemoveDynamic(Probe,&UAskirTransactionPersistenceProbe::ObserveReputationChange);
    if(!TestTrue(TEXT("Canonical acceptance transaction"),bAccepted))return false;
    TestTrue(TEXT("Faction callback executed inside real acceptance"),Probe->bCallbackObserved);
    TestTrue(TEXT("Acceptance callback cannot save partial receipts"),Probe->bCaptureBlocked);
    TestTrue(TEXT("Acceptance callback cannot restore over its own transaction"),Probe->bRestoreBlocked);
    Patch->RefreshMissionState();
    TestFalse(TEXT("Unexplored site remains visible"),Patch->IsHidden());
    TStrongObjectPtr<UAskirSaveGame> BeforeExplore(Fixture.Mode->CaptureSnapshot());
    if(!TestNotNull(TEXT("Active exploration snapshot"),BeforeExplore.Get()))return false;
    if(!TestTrue(TEXT("Canonical meadow exploration"),Fixture.Player->HerbMission->ExploreHerbs(TEXT("SQ003_Meadow_V84"))))return false;
    Patch->RefreshMissionState();
    TestTrue(TEXT("Explored site hides in outgoing timeline"),Patch->IsHidden());
    TStrongObjectPtr<UAskirSaveGame> Explored(Fixture.Mode->CaptureSnapshot());
    if(!TestNotNull(TEXT("Complete return-stage snapshot"),Explored.Get()))return false;
    FString Reason;TestTrue(TEXT("Return stage validates"),Explored->ValidateSnapshot(Reason));
    TArray<uint8> Bytes;
    if(!TestTrue(TEXT("Serialize actual complete save"),UGameplayStatics::SaveGameToMemory(Explored.Get(),Bytes)))return false;
    TStrongObjectPtr<UAskirSaveGame> Loaded(Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes)));
    if(!TestNotNull(TEXT("Deserialize actual complete save"),Loaded.Get()))return false;
    TestTrue(TEXT("Unique meadow identity survives serialization"),Loaded->SavedConsumedQuestTriggers.Contains(TEXT("SQ_003_EXPLORED_SOURCE_SQ003_Meadow_V84")));
    TestEqual(TEXT("Save format remains five"),Loaded->SaveFormatVersion,5);
    if(!TestTrue(TEXT("Restore earlier unexplored timeline"),Fixture.Mode->RestoreSnapshot(BeforeExplore.Get(),false)))return false;
    TestEqual(TEXT("Earlier mission stage restored"),Fixture.Player->HerbMission->GetStage(),EAskirHerbStage::ExploreMeadow);
    TestFalse(TEXT("Restore refreshes real patch visibility without manual actor refresh"),Patch->IsHidden());
    TestTrue(TEXT("Restore reenables patch collision"),Patch->GetActorEnableCollision());
    Fixture.Player->DialogueComponent->RestoreFlags({});Reputation->SetReputation(TEXT("Freie Händler"),-70);
    if(!TestTrue(TEXT("Load explored timeline"),Fixture.Mode->RestoreSnapshot(Loaded.Get(),false)))return false;
    TestTrue(TEXT("Repeated load is idempotent"),Fixture.Mode->RestoreSnapshot(Loaded.Get(),false));
    TestEqual(TEXT("Stage derives from authoritative saved receipts"),Fixture.Player->HerbMission->GetStage(),EAskirHerbStage::ReturnToRurik);
    TestEqual(TEXT("Bargain choice restored"),Fixture.Player->HerbMission->GetAcceptedChoice(),FName(TEXT("bargain")));
    TestEqual(TEXT("Reputation restored without awarding choice again"),Reputation->GetReputation(TEXT("Freie Händler")),2);
    TestTrue(TEXT("Restored exploration hides native patch before completion"),Patch->IsHidden());
    TestFalse(TEXT("Restored site cannot remain interactive"),Patch->GetActorEnableCollision());
    TestFalse(TEXT("Restored meadow cannot be explored twice"),Fixture.Player->HerbMission->ExploreHerbs(TEXT("SQ003_Meadow_V84")));
    TestEqual(TEXT("Loading unexplained progress grants no gold"),Fixture.Player->TradeComponent->Gold,0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirHerbSnapshotConsistencyTest,"Askir.Persistence.HerbRejectsForgedReceiptsAndObjectiveMirrors",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirHerbSnapshotConsistencyTest::RunTest(const FString& Parameters)
{
    FString Reason;auto* Legacy=NewObject<UAskirSaveGame>();
    TestTrue(TEXT("Existing format-five save without herb quest is unchanged"),Legacy->ValidateSnapshot(Reason));
    Legacy->SavedDialogueFlags.Add(TEXT("SQ_003_STAGE_COMPLETED"));
    TestTrue(TEXT("Virtual stage-looking lore has no authority over receipt validation"),Legacy->ValidateSnapshot(Reason));
    auto* Save=HerbSaveFixture();
    if(!TestTrue(TEXT("Canonical return-stage fixture validates"),Save->ValidateSnapshot(Reason)))return false;
    const auto Reject=[this,&Reason](UAskirSaveGame* Invalid,const TCHAR* Label){TestFalse(Label,Invalid->ValidateSnapshot(Reason));};
    auto* Invalid=HerbSaveFixture();Invalid->SavedDialogueFlags.Add(TEXT("SQ_003_CHOICE_bargain"));Reject(Invalid,TEXT("Conflicting accepted choices rejected"));
    Invalid=HerbSaveFixture();Invalid->SavedDialogueFlags.Reset();Reject(Invalid,TEXT("Accepted receipt requires its choice"));
    Invalid=HerbSaveFixture();Invalid->SavedConsumedQuestTriggers.Remove(TEXT("SQ_003_ACCEPTED"));Reject(Invalid,TEXT("Exploration cannot predate acceptance"));
    Invalid=HerbSaveFixture();Invalid->SavedConsumedQuestTriggers.Remove(TEXT("SQ_003_EXPLORED"));Reject(Invalid,TEXT("Source receipt requires exploration receipt"));
    Invalid=HerbSaveFixture();Invalid->SavedConsumedQuestTriggers.Remove(TEXT("SQ_003_EXPLORED_SOURCE_SQ003_Meadow_V84"));Reject(Invalid,TEXT("Exploration needs its canonical source"));
    Invalid=HerbSaveFixture();Invalid->SavedConsumedQuestTriggers.Add(TEXT("SQ_003_EXPLORED_SOURCE_Unknown"));Reject(Invalid,TEXT("Additional unknown source cannot inflate exploration"));
    Invalid=HerbSaveFixture();Invalid->SavedConsumedQuestTriggers.Add(TEXT("SQ_003_UNKNOWN"));Reject(Invalid,TEXT("Unknown reserved mission receipt rejected"));
    Invalid=HerbSaveFixture();Invalid->SavedClaimedRewards.Add(TEXT("SQ_003"));Reject(Invalid,TEXT("Claimed reward cannot belong to an active herb quest"));
    Invalid=HerbSaveFixture();Invalid->SavedQuestStates[TEXT("SQ_003")]=EAskirQuestState::Failed;Invalid->SavedObjectivesByQuest.Reset();Invalid->SavedActiveObjectives.Reset();Invalid->SavedLegacyActiveQuestID=NAME_None;Reject(Invalid,TEXT("Accepted herb mission cannot silently become failed"));
    Invalid=HerbSaveFixture();Invalid->SavedObjectivesByQuest[TEXT("SQ_003")].Objectives[0].CurrentCount=0;Invalid->SavedActiveObjectives[0].CurrentCount=0;Reject(Invalid,TEXT("Matching objective mirrors cannot erase the exploration receipt"));
    Invalid=HerbSaveFixture();Invalid->SavedObjectivesByQuest[TEXT("SQ_003")].Objectives[1].CurrentCount=1;Invalid->SavedActiveObjectives[1].CurrentCount=1;Reject(Invalid,TEXT("Unpaid return objective cannot appear completed"));
    Invalid=HerbSaveFixture();Invalid->SavedObjectivesByQuest[TEXT("SQ_003")].Objectives[0].RequiredCount=2;Invalid->SavedActiveObjectives[0].RequiredCount=2;Reject(Invalid,TEXT("Canonical exploration count cannot be increased by a save"));
    Invalid=HerbSaveFixture();Invalid->SavedObjectivesByQuest[TEXT("SQ_003")].Objectives[0].ObjectiveID=TEXT("OTHER");Invalid->SavedActiveObjectives[0].ObjectiveID=TEXT("OTHER");Reject(Invalid,TEXT("Matching mirrors cannot rename canonical objectives"));
    Save->SavedQuestStates[TEXT("SQ_003")]=EAskirQuestState::Completed;Save->SavedObjectivesByQuest.Reset();Save->SavedActiveObjectives.Reset();Save->SavedLegacyActiveQuestID=NAME_None;
    TestFalse(TEXT("Completion without reward receipt rejected"),Save->ValidateSnapshot(Reason));
    Save->SavedClaimedRewards.Add(TEXT("SQ_003"));
    TestTrue(TEXT("Canonical completed paid quest validates"),Save->ValidateSnapshot(Reason));
    Save->SavedConsumedQuestTriggers.Remove(TEXT("SQ_003_EXPLORED_SOURCE_SQ003_Meadow_V84"));
    TestFalse(TEXT("Completion cannot discard its exploration source"),Save->ValidateSnapshot(Reason));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirHerbRewardPersistenceTest,"Askir.Persistence.HerbRewardSerializedReloadNeverPaysTwice",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirHerbRewardPersistenceTest::RunTest(const FString& Parameters)
{
    FHerbPersistenceFixture Fixture;if(!TestTrue(TEXT("Native player owns herb mission and save mode"),Fixture.Ready()))return false;
    UAskirItemData* Item=Fixture.PrepareReward();if(!TestNotNull(TEXT("Canonical test reward in a valid game package"),Item))return false;
    if(!TestTrue(TEXT("Accept herb quest"),Fixture.Player->HerbMission->AcceptHerbs(TEXT("help"))))return false;
    if(!TestTrue(TEXT("Explore canonical site"),Fixture.Player->HerbMission->ExploreHerbs(TEXT("SQ003_Meadow_V84"))))return false;
    auto* Probe=NewObject<UAskirTransactionPersistenceProbe>();Probe->Mode=Fixture.Mode;Probe->Snapshot=Fixture.Mode->CaptureSnapshot();
    if(!TestNotNull(TEXT("Pre-reward snapshot"),Probe->Snapshot.Get()))return false;
    Fixture.Player->InventoryComponent->OnInventoryChanged.AddDynamic(Probe,&UAskirTransactionPersistenceProbe::ObserveInventoryChange);
    const bool bPaid=Fixture.Player->HerbMission->ReturnToRurik();
    Fixture.Player->InventoryComponent->OnInventoryChanged.RemoveDynamic(Probe,&UAskirTransactionPersistenceProbe::ObserveInventoryChange);
    if(!TestTrue(TEXT("Actual atomic turn-in succeeds"),bPaid))return false;
    TestTrue(TEXT("Reward emits real inventory delegate"),Probe->bCallbackObserved);
    TestTrue(TEXT("Reward callback cannot capture open transaction"),Probe->bCaptureBlocked);
    TestTrue(TEXT("Reward callback cannot rewind committed receipt"),Probe->bRestoreBlocked);
    TestEqual(TEXT("Exact canonical gold paid"),Fixture.Player->TradeComponent->Gold,24);
    TestEqual(TEXT("Exact canonical reward item paid"),Fixture.Player->InventoryComponent->GetItemCount(Item),1);
    TestEqual(TEXT("Canonical 150 XP crosses level-one threshold"),Fixture.Player->ProgressionComponent->Progress.Level,2);
    TestEqual(TEXT("Remaining canonical experience"),Fixture.Player->ProgressionComponent->Progress.Experience,50);
    TStrongObjectPtr<UAskirSaveGame> Complete(Fixture.Mode->CaptureSnapshot());
    if(!TestNotNull(TEXT("Paid reward snapshot"),Complete.Get()))return false;
    for(int32 Repeat=0;Repeat<3;++Repeat)
    {
        TArray<uint8> Bytes;
        if(!TestTrue(TEXT("Serialize paid quest again"),UGameplayStatics::SaveGameToMemory(Complete.Get(),Bytes)))return false;
        TStrongObjectPtr<UAskirSaveGame> Loaded(Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes)));
        if(!TestNotNull(TEXT("Deserialize paid quest again"),Loaded.Get()))return false;
        Fixture.Player->TradeComponent->Gold=999;
        if(!TestTrue(TEXT("Restore complete paid state"),Fixture.Mode->RestoreSnapshot(Loaded.Get(),false)))return false;
        TestEqual(TEXT("Complete stage remains complete"),Fixture.Player->HerbMission->GetStage(),EAskirHerbStage::Completed);
        TestFalse(TEXT("Loaded reward cannot be paid twice"),Fixture.Player->HerbMission->ReturnToRurik());
        TestEqual(TEXT("Repeated save/load keeps exact gold"),Fixture.Player->TradeComponent->Gold,24);
        TestEqual(TEXT("Repeated save/load keeps exactly one item"),Fixture.Player->InventoryComponent->GetItemCount(Item),1);
        TestEqual(TEXT("Repeated save/load does not award more XP"),Fixture.Player->ProgressionComponent->Progress.Experience,50);
        Complete.Reset(Fixture.Mode->CaptureSnapshot());
        if(!TestNotNull(TEXT("Recapture completed ledger"),Complete.Get()))return false;
    }
    return true;
}
#endif
