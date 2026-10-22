#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Tests/AskirHerbMissionProbe.h"
#include "Characters/AskirCharacter.h"
#include "Combat/AskirCombatComponent.h"
#include "Mission/AskirHerbMissionComponent.h"
#include "Mission/AskirHerbPatchActor.h"
#include "NPC/AskirDialogueComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Trade/AskirTradeComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Social/AskirReputationComponent.h"
#include "Game/AskirGameMode.h"
#include "Game/AskirPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/BoxComponent.h"
#include "UObject/Package.h"
#include <limits>

namespace
{
struct FHerbFixture : FAskirAutomationTestWorld
{
    AAskirCharacter* Player=nullptr;
    APlayerController* Controller=nullptr;
    UAskirHerbMissionComponent* Mission=nullptr;
    UAskirItemData* Reward=nullptr;
    FHerbFixture()
    {
        if(!World)return;
        Player=Spawn<AAskirCharacter>(FVector(0.,0.,100.));Controller=Spawn<APlayerController>();
        if(!Player||!Controller)return;
        Controller->Possess(Player);Player->GetCharacterMovement()->DisableMovement();
        Mission=Player->FindComponentByClass<UAskirHerbMissionComponent>();
        if(!Mission)return;
        auto* Package=CreatePackage(*(FString(TEXT("/Game/Askir/Automation/Herbs_"))+FGuid::NewGuid().ToString(EGuidFormats::Digits)));
        Reward=NewObject<UAskirItemData>(Package,TEXT("item_004"),RF_Public);Reward->ItemID=TEXT("item_004");Reward->MaxStackSize=99;
        auto* Data=Mission->ResolveQuestData();if(!Data)return;Data->RewardItem=Reward;
        Player->InventoryComponent->Items.Reset();Player->TradeComponent->Gold=0;
        Player->ProgressionComponent->RestoreProgress(FAskirProgressionSnapshot());
    }
    bool Ready() const {return World&&Player&&Controller&&Mission&&Reward&&Mission->QuestData;}
    AAskirHerbPatchActor* Patch()
    {
        auto* Result=Spawn<AAskirHerbPatchActor>(FVector(180.,0.,100.));
        if(Result){Result->MissionPlayer=Player;Result->BindMissionEvents();}return Result;
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirHerbLifecycleTest,"Askir.Mission.Herbs.CanonicalExplorationAndSingleReward",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirHerbLifecycleTest::RunTest(const FString& Parameters)
{
    FHerbFixture F;if(!TestTrue(TEXT("Possessed player owns native herb mission"),F.Ready()))return false;
    const FName Source=UAskirHerbMissionComponent::GetCanonicalSourceID();
    TestFalse(TEXT("Exploration cannot start a quest implicitly"),F.Mission->ExploreHerbs(Source));
    TestFalse(TEXT("Unknown acceptance choice grants nothing"),F.Mission->AcceptHerbs(TEXT("unknown")));
    TestTrue(TEXT("Canonical help choice starts the quest"),F.Mission->AcceptHerbs(TEXT("help")));
    TestEqual(TEXT("Accepted stage"),F.Mission->GetStage(),EAskirHerbStage::ExploreMeadow);
    TestEqual(TEXT("Help faction receives exactly two"),F.Player->ReputationComponent->GetReputation(TEXT("Waldhüter")),2);
    TestFalse(TEXT("Acceptance cannot change choice or repeat reputation"),F.Mission->AcceptHerbs(TEXT("bargain")));
    TestFalse(TEXT("Only the authored source can grant exploration"),F.Mission->ExploreHerbs(TEXT("OtherMeadow")));
    TestTrue(TEXT("Authored meadow records exploration"),F.Mission->ExploreHerbs(Source));
    TestFalse(TEXT("Exploration receipt is idempotent"),F.Mission->ExploreHerbs(Source));
    TestTrue(TEXT("Canonical source is persisted"),F.Mission->HasExploredHerbs(Source));
    TestEqual(TEXT("Exploration pays no gold early"),F.Player->TradeComponent->Gold,0);
    TestEqual(TEXT("Exploration gives no pickup item early"),F.Player->InventoryComponent->GetItemCount(F.Reward),0);
    TestTrue(TEXT("Report commits actual quest rewards"),F.Mission->ReturnToRurik());
    TestEqual(TEXT("Exact gold once"),F.Player->TradeComponent->Gold,24);
    TestEqual(TEXT("150XP crosses first threshold"),F.Player->ProgressionComponent->Progress.Level,2);
    TestEqual(TEXT("Remaining XP after level-up"),F.Player->ProgressionComponent->Progress.Experience,50);
    TestEqual(TEXT("One canonical material reward"),F.Player->InventoryComponent->GetItemCount(F.Reward),1);
    TestEqual(TEXT("Completion follows journal"),F.Mission->GetStage(),EAskirHerbStage::Completed);
    TestTrue(TEXT("Generic reward receipt persisted"),F.Player->QuestComponent->HasClaimedReward(TEXT("SQ_003")));
    TestFalse(TEXT("Repeated report cannot pay again"),F.Mission->ReturnToRurik());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirHerbRewardRetryTest,"Askir.Mission.Herbs.FullBagOverflowAndMissingRewardAreRetryable",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirHerbRewardRetryTest::RunTest(const FString& Parameters)
{
    FHerbFixture F;if(!TestTrue(TEXT("Possessed native mission"),F.Ready()))return false;
    TestTrue(TEXT("Accept bargain"),F.Mission->AcceptHerbs(TEXT("bargain")));
    TestTrue(TEXT("Explore"),F.Mission->ExploreHerbs(UAskirHerbMissionComponent::GetCanonicalSourceID()));
    auto* Filler=NewObject<UAskirItemData>();Filler->ItemID=TEXT("V84_FILLER");
    F.Player->InventoryComponent->MaxInventorySlots=1;TestTrue(TEXT("Fill only inventory slot"),F.Player->InventoryComponent->AddItem(Filler,1));
    TestFalse(TEXT("Full bag rejects entire payout"),F.Mission->ReturnToRurik());
    TestEqual(TEXT("Gold untouched"),F.Player->TradeComponent->Gold,0);
    TestEqual(TEXT("XP untouched"),F.Player->ProgressionComponent->Progress.Experience,0);
    TestFalse(TEXT("No false claimed receipt"),F.Player->QuestComponent->HasClaimedReward(TEXT("SQ_003")));
    const auto Objectives=F.Player->QuestComponent->GetObjectivesForQuest(TEXT("SQ_003"));
    if(!TestEqual(TEXT("Canonical active objective count"),Objectives.Num(),2))return false;
    TestEqual(TEXT("Return mirror rolled back"),Objectives[1].CurrentCount,0);
    F.Player->InventoryComponent->RemoveItem(Filler,1);F.Player->TradeComponent->Gold=MAX_int32-23;
    TestFalse(TEXT("Wallet overflow rejects payout"),F.Mission->ReturnToRurik());
    TestEqual(TEXT("No item on overflow"),F.Player->InventoryComponent->GetItemCount(F.Reward),0);
    F.Player->TradeComponent->Gold=0;F.Player->ProgressionComponent->Progress.UnspentPoints=MAX_int32;
    TestFalse(TEXT("Level-point overflow rejects entire payout"),F.Mission->ReturnToRurik());
    TestEqual(TEXT("No gold on XP failure"),F.Player->TradeComponent->Gold,0);
    F.Player->ProgressionComponent->Progress.UnspentPoints=0;
    auto* Data=F.Mission->QuestData.Get();Data->RewardItem=NewObject<UAskirItemData>();Data->RewardItem->ItemID=TEXT("wrong_item");
    TestFalse(TEXT("Noncanonical reward rejects payout"),F.Mission->ReturnToRurik());
    Data->RewardItem=F.Reward;F.Player->TradeComponent->Gold=MAX_int32-24;
    TestTrue(TEXT("Canonical reward and exact wallet boundary can retry"),F.Mission->ReturnToRurik());
    TestEqual(TEXT("Exact wallet boundary"),F.Player->TradeComponent->Gold,MAX_int32);
    TestEqual(TEXT("Canonical reward once"),F.Player->InventoryComponent->GetItemCount(F.Reward),1);
    TestEqual(TEXT("Bargain reputation once"),F.Player->ReputationComponent->GetReputation(TEXT("Freie Händler")),2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirHerbInteractionTest,"Askir.Mission.Herbs.RealPatchRequiresContextSightAndExplicitInteraction",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirHerbInteractionTest::RunTest(const FString& Parameters)
{
    FHerbFixture F;if(!TestTrue(TEXT("Possessed native mission"),F.Ready()))return false;
    auto* Patch=F.Patch();if(!TestNotNull(TEXT("Native patch"),Patch))return false;
    TestFalse(TEXT("Patch cannot explore before acceptance"),Patch->CanInteractWith(F.Player));
    F.Mission->AcceptHerbs(TEXT("help"));
    TestFalse(TEXT("Nearby patch alone does not grant receipt"),F.Mission->HasExploredHerbs(Patch->SourceID));
    TestTrue(TEXT("Nearby controlled player can inspect"),Patch->CanInteractWith(F.Player));
    Patch->InteractionRange=std::numeric_limits<float>::quiet_NaN();TestFalse(TEXT("NaN range rejected"),Patch->CanInteractWith(F.Player));
    Patch->InteractionRange=240.f;Patch->SetActorLocation(FVector(1000.,0.,100.));TestFalse(TEXT("Remote context rejected"),Patch->CanInteractWith(F.Player));
    Patch->SetActorLocation(FVector(180.,0.,100.));
    auto* Wall=F.Spawn<AActor>(FVector(90.,0.,100.));auto* Box=F.AddComponent<UBoxComponent>(Wall);Wall->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(15.,100.,160.));Box->SetCollisionProfileName(TEXT("BlockAll"));Wall->SetActorLocation(FVector(90.,0.,100.));
    TestFalse(TEXT("Current blocking wall rejects interaction"),Patch->CanInteractWith(F.Player));
    Wall->SetActorEnableCollision(false);
    TestTrue(TEXT("Explicit interact records exploration"),IAskirInteractable::Execute_Interact(Patch,F.Player));
    TestTrue(TEXT("Collected investigation marker hidden"),Patch->IsHidden());
    TestFalse(TEXT("Collected marker has no collision"),Patch->GetActorEnableCollision());
    TestFalse(TEXT("Hidden marker cannot replay interaction"),IAskirInteractable::Execute_Interact(Patch,F.Player));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirHerbReentryTest,"Askir.Mission.Herbs.RealDelegatesBlockReentryAndPartialSnapshots",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirHerbReentryTest::RunTest(const FString& Parameters)
{
    FHerbFixture F;if(!TestTrue(TEXT("Possessed native mission"),F.Ready()))return false;
    auto* Mode=F.Spawn<AAskirGameMode>();if(!TestNotNull(TEXT("Mode"),Mode))return false;
    auto* Probe=NewObject<UAskirHerbMissionProbe>();Probe->Mission=F.Mission;Probe->Mode=Mode;Probe->Snapshot=Mode->CaptureSnapshot();
    if(!TestNotNull(TEXT("Consistent baseline snapshot"),Probe->Snapshot.Get()))return false;
    F.Player->ReputationComponent->OnReputationChanged.AddDynamic(Probe,&UAskirHerbMissionProbe::ObserveReputationChange);
    F.Player->QuestComponent->OnQuestCompleted.AddDynamic(Probe,&UAskirHerbMissionProbe::ObserveQuestCompleted);
    TestTrue(TEXT("Acceptance survives actual listener"),F.Mission->AcceptHerbs(TEXT("help")));
    TestTrue(TEXT("Real reputation delegate fired"),Probe->bObservedAcceptance);
    TestTrue(TEXT("Mission guard visible inside callback"),Probe->bGuardVisible);
    TestTrue(TEXT("Receipt precedes external callback"),Probe->bReceiptCommittedBeforeCallback);
    TestFalse(TEXT("Reentrant acceptance rejected"),Probe->bReentrantAccept);
    TestFalse(TEXT("Reentrant exploration rejected"),Probe->bReentrantExplore);
    TestTrue(TEXT("Acceptance blocks snapshot capture"),Probe->bAcceptanceCaptureBlocked);
    TestTrue(TEXT("Acceptance blocks snapshot restore"),Probe->bAcceptanceRestoreBlocked);
    F.Mission->ExploreHerbs(UAskirHerbMissionComponent::GetCanonicalSourceID());
    TestTrue(TEXT("Return completes"),F.Mission->ReturnToRurik());
    TestTrue(TEXT("Actual completion delegate fired"),Probe->bObservedCompletion);
    TestFalse(TEXT("Reentrant return rejected"),Probe->bReentrantReturn);
    TestTrue(TEXT("Reward callback blocks partial snapshot capture"),Probe->bCompletionCaptureBlocked);
    TestFalse(TEXT("Mission guard released after transaction"),F.Mission->IsTransactionInProgress());
    F.Player->ReputationComponent->OnReputationChanged.RemoveDynamic(Probe,&UAskirHerbMissionProbe::ObserveReputationChange);
    F.Player->QuestComponent->OnQuestCompleted.RemoveDynamic(Probe,&UAskirHerbMissionProbe::ObserveQuestCompleted);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirHerbExplorationDestructionTest,
    "Askir.Mission.Herbs.ExplorationCallbackDestructionCannotRevivePatchBindings",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirHerbExplorationDestructionTest::RunTest(const FString& Parameters)
{
    for(int32 Scenario=0;Scenario<2;++Scenario)
    {
        FHerbFixture F;if(!TestTrue(TEXT("Possessed native mission"),F.Ready()))return false;
        auto* Patch=F.Patch();if(!TestNotNull(TEXT("Native patch"),Patch))return false;
        if(!TestTrue(TEXT("Accept before exploration"),F.Mission->AcceptHerbs(TEXT("help"))))return false;
        auto* Journal=F.Player->QuestComponent.Get();
        auto* Probe=NewObject<UAskirHerbMissionProbe>();Probe->Mission=F.Mission;Probe->Patch=Patch;
        Probe->ExplorationActorToDestroy=Scenario==0?static_cast<AActor*>(Patch):static_cast<AActor*>(F.Player);
        Journal->OnQuestProgressChanged.AddDynamic(Probe,&UAskirHerbMissionProbe::ObserveExplorationChanged);
        TestTrue(TEXT("Exploration remains committed when its callback destroys context"),IAskirInteractable::Execute_Interact(Patch,F.Player));
        TestTrue(TEXT("Actual exploration delegate executed"),Probe->bObservedExploration);
        TestTrue(TEXT("Durable exploration receipt precedes destruction callback"),Probe->bExplorationReceiptCommitted);
        TestTrue(TEXT("Callback destroys its selected actor"),Probe->bExplorationDestroyAccepted);
        TestFalse(TEXT("Destroying patch or player cannot bind a journal again"),Probe->bPatchRebindAccepted);
        TestTrue(TEXT("Refresh returns without recursive rebinding"),Probe->bPatchRefreshReturned);
        TestNull(TEXT("Old interaction continuation cannot reinstall a destroyed context"),Patch->MissionPlayer.Get());
        TestFalse(TEXT("Destroyed context cannot leave a quest delegate behind"),
            Journal->OnQuestProgressChanged.Contains(Patch,FName(TEXT("OnQuestProgressChanged"))));
        Patch->RefreshMissionState();Patch->RefreshMissionState();
        TestFalse(TEXT("Repeated bind remains closed after context destruction"),Patch->BindMissionEvents());
        Journal->OnQuestProgressChanged.RemoveDynamic(Probe,&UAskirHerbMissionProbe::ObserveExplorationChanged);
    }
    return true;
}
#endif
