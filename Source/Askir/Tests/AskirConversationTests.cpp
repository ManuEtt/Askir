#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Tests/AskirConversationProbe.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Combat/AskirCombatComponent.h"
#include "Dialogue/AskirConversationComponent.h"
#include "Dialogue/AskirConversationCatalog.h"
#include "NPC/AskirDialogueComponent.h"
#include "NPC/AskirNPCInteractionComponent.h"
#include "Mission/AskirDeliveryMissionComponent.h"
#include "Mission/AskirDeliveryQuestActor.h"
#include "Quest/AskirQuestComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Trade/AskirTradeComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Social/AskirReputationComponent.h"
#include "Save/AskirSaveGame.h"
#include "Game/AskirGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Package.h"

namespace
{
struct FConversationFixture : FAskirAutomationTestWorld
{
    AAskirCharacter* Player=nullptr;
    APlayerController* Controller=nullptr;
    UAskirConversationComponent* Conversation=nullptr;
    FConversationFixture()
    {
        if(!World)return;
        Player=Spawn<AAskirCharacter>(FVector(0.,0.,96.));Controller=Spawn<APlayerController>();
        if(!Player||!Controller)return;
        Controller->Possess(Player);Controller->SetControlRotation(FRotator::ZeroRotator);
        Player->GetCharacterMovement()->DisableMovement();
        Conversation=Player->FindComponentByClass<UAskirConversationComponent>();
    }
    bool Ready() const {return World&&Player&&Controller&&Conversation;}
    AAskirNPC* Resident()
    {
        AAskirNPC* NPC=Spawn<AAskirNPC>(FVector(180.,0.,96.));
        if(NPC){NPC->bHostile=false;NPC->GetCharacterMovement()->DisableMovement();}return NPC;
    }
    AAskirDeliveryQuestActor* Torben()
    {
        auto* Giver=Spawn<AAskirDeliveryQuestActor>(FVector(180.,0.,96.));
        if(Giver){Giver->Role=EAskirDeliveryQuestRole::Giver;Giver->MissionPlayer=Player;}return Giver;
    }
    UAskirItemData* Reward()
    {
        UPackage* Package=CreatePackage(*(FString(TEXT("/Game/Askir/Automation/Conversation_"))+FGuid::NewGuid().ToString(EGuidFormats::Digits)));
        auto* Item=NewObject<UAskirItemData>(Package,TEXT("item_002"),RF_Public);Item->ItemID=TEXT("item_002");Item->MaxStackSize=99;
        auto* Quest=NewObject<UAskirQuestData>(Player);Quest->QuestID=TEXT("SQ_001");Quest->GoldReward=16;Quest->ExperienceReward=100;
        Quest->RewardItem=Item;Quest->RewardItemQuantity=1;
        const FName IDs[]={TEXT("SQ_001_A"),TEXT("SQ_001_CARGO"),TEXT("SQ_001_B")};
        for(int32 I=0;I<3;++I){FAskirQuestObjective Objective;Objective.ObjectiveID=IDs[I];Objective.RequiredCount=I==0?2:1;Quest->Objectives.Add(Objective);}
        Player->DeliveryMission->QuestData=Quest;Player->InventoryComponent->Items.Reset();Player->TradeComponent->Gold=0;
        Player->ProgressionComponent->RestoreProgress(FAskirProgressionSnapshot());return Item;
    }
};
FAskirDialogueNode LegacyStart()
{
    FAskirDialogueNode Node;Node.NodeID=TEXT("Start");Node.Speaker=FText::FromString(TEXT("Resident"));Node.Text=FText::FromString(TEXT("Eine Erinnerung."));
    FAskirDialogueChoice Choice;Choice.Label=FText::FromString(TEXT("Erzähl mir davon."));Choice.NextNode=TEXT("Known");Choice.GrantedFlag=TEXT("V83_LORE");Node.Choices.Add(Choice);return Node;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirConversationLegacyTest,"Askir.Dialogue.Conversation.LegacyGraphPlayerFlagsAndStaleRevision",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirConversationLegacyTest::RunTest(const FString& Parameters)
{
    FConversationFixture Fixture;if(!TestTrue(TEXT("Controlled player owns conversation backend"),Fixture.Ready()))return false;
    auto* Resident=Fixture.Resident();auto* Local=Resident->FindComponentByClass<UAskirDialogueComponent>();
    Local->Nodes.Add(LegacyStart());FAskirDialogueNode Known;Known.NodeID=TEXT("Known");Known.Text=FText::FromString(TEXT("Die Erinnerung bleibt."));
    FAskirDialogueChoice Finish;Finish.Label=FText::FromString(TEXT("Verstanden."));Finish.RequiredFlag=TEXT("V83_LORE");Known.Choices.Add(Finish);Local->Nodes.Add(Known);
    TestTrue(TEXT("Legacy speaker graph can begin"),Fixture.Conversation->BeginConversation(Resident));
    const int64 First=Fixture.Conversation->GetRevision();
    TestTrue(TEXT("Beginning a graph reserves a nonzero token"),First>0);
    TestFalse(TEXT("A second session cannot replace an active cursor"),Fixture.Conversation->BeginConversation(Resident));
    TestTrue(TEXT("Graph choice follows its authored branch"),Fixture.Conversation->SelectChoice(TEXT("LegacyChoice_0"),First));
    TestEqual(TEXT("Next node reached"),Fixture.Conversation->GetCurrentNode().NodeID,FName(TEXT("Known")));
    TestTrue(TEXT("Lore belongs to the player's persisted ledger"),Fixture.Player->DialogueComponent->HasFlag(TEXT("V83_LORE")));
    TestFalse(TEXT("NPC-local ledger cannot substitute for player flags"),Local->HasFlag(TEXT("V83_LORE")));
    TestEqual(TEXT("New player flag reveals conditional reply"),Fixture.Conversation->GetAvailableChoices().Num(),1);
    TestFalse(TEXT("Old button cannot select same-ID reply in a new node"),Fixture.Conversation->SelectChoice(TEXT("LegacyChoice_0"),First));
    TestTrue(TEXT("Fresh token closes terminal reply"),Fixture.Conversation->SelectChoice(TEXT("LegacyChoice_0"),Fixture.Conversation->GetRevision()));
    TestFalse(TEXT("Terminal choice releases the cursor"),Fixture.Conversation->IsActive());
    const int64 Ended=Fixture.Conversation->GetRevision();
    TestTrue(TEXT("A later session can begin"),Fixture.Conversation->BeginConversation(Resident));
    TestTrue(TEXT("A new session never repeats a completed-session token"),Fixture.Conversation->GetRevision()>Ended);
    Fixture.Conversation->EndConversation();
    Local->Nodes[0].Choices[0].GrantedFlag=TEXT("SQ_001_CHOICE_help");
    TestFalse(TEXT("Legacy lore cannot forge protected mission receipts"),Fixture.Conversation->BeginConversation(Resident));
    TestFalse(TEXT("Rejected graph cannot grant a protected flag"),Fixture.Player->DialogueComponent->HasFlag(TEXT("SQ_001_CHOICE_help")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirConversationDeliveryTest,"Askir.Dialogue.Conversation.CanonicalDeliveryChoiceReturnAndFlagSerialization",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirConversationDeliveryTest::RunTest(const FString& Parameters)
{
    FConversationFixture Fixture;if(!TestTrue(TEXT("Controlled player owns conversation backend"),Fixture.Ready()))return false;
    UAskirItemData* Reward=Fixture.Reward();auto* Torben=Fixture.Torben();auto* Mode=Fixture.Spawn<AAskirGameMode>();
    TestTrue(TEXT("Canonical catalog resolves and validates"),UAskirConversationComponent::IsKnownDialogue(TEXT("D_SQ_001")));
    TestFalse(TEXT("Unknown catalog ID is rejected"),UAskirConversationComponent::IsKnownDialogue(TEXT("NO_SUCH_DIALOGUE")));
    TestTrue(TEXT("Torben graph begins at canonical opening"),Fixture.Conversation->BeginConversation(Torben,TEXT("D_SQ_001")));
    TestTrue(TEXT("Optional authored lore branch executes"),Fixture.Conversation->SelectChoice(TEXT("road_history"),Fixture.Conversation->GetRevision()));
    TestTrue(TEXT("Lore branch records persistent player memory"),Fixture.Player->DialogueComponent->HasFlag(TEXT("TORBEN_ROAD_HISTORY")));
    TestTrue(TEXT("Lore branch returns to delivery choice"),Fixture.Conversation->SelectChoice(TEXT("back"),Fixture.Conversation->GetRevision()));
    TestTrue(TEXT("Canonical bargain action calls real mission acceptance"),Fixture.Conversation->SelectChoice(TEXT("bargain"),Fixture.Conversation->GetRevision()));
    TestEqual(TEXT("Domain mission stage advanced"),Fixture.Player->DeliveryMission->GetStage(),EAskirDeliveryStage::DefeatBandits);
    TestEqual(TEXT("Domain reputation applied once"),Fixture.Player->ReputationComponent->GetReputation(TEXT("Freie Händler")),2);
    Fixture.Conversation->EndConversation();
    TestTrue(TEXT("Reopening resolves current quest-stage entry"),Fixture.Conversation->BeginConversation(Torben,TEXT("D_SQ_001")));
    TestEqual(TEXT("Accepted quest uses progress branch"),Fixture.Conversation->GetCurrentNode().NodeID,FName(TEXT("active")));
    Fixture.Conversation->EndConversation();
    TestTrue(TEXT("First encounter receipt"),Fixture.Player->DeliveryMission->CreditBandit(TEXT("V83_A")));
    TestTrue(TEXT("Second encounter receipt"),Fixture.Player->DeliveryMission->CreditBandit(TEXT("V83_B")));
    TestTrue(TEXT("Cargo receipt"),Fixture.Player->DeliveryMission->RecoverCargo(TEXT("V83_CARGO")));
    TestTrue(TEXT("Return dialogue begins"),Fixture.Conversation->BeginConversation(Torben,TEXT("D_SQ_001")));
    TestEqual(TEXT("Return stage chooses turn-in branch"),Fixture.Conversation->GetCurrentNode().NodeID,FName(TEXT("return")));
    const int64 TurnInToken=Fixture.Conversation->GetRevision();
    TestTrue(TEXT("Turn-in choice commits real quest rewards"),Fixture.Conversation->SelectChoice(TEXT("turn_in"),TurnInToken));
    TestEqual(TEXT("Domain reward pays exact gold"),Fixture.Player->TradeComponent->Gold,16);
    TestEqual(TEXT("Domain reward grants one actual item"),Fixture.Player->InventoryComponent->GetItemCount(Reward),1);
    TestFalse(TEXT("Replayed button cannot grant a second reward"),Fixture.Conversation->SelectChoice(TEXT("turn_in"),TurnInToken));
    Fixture.Conversation->EndConversation();
    TestTrue(TEXT("Completed quest can be discussed again"),Fixture.Conversation->BeginConversation(Torben,TEXT("D_SQ_001")));
    TestEqual(TEXT("Persisted bargain selects authored ending response"),Fixture.Conversation->GetCurrentNode().NodeID,FName(TEXT("thanks_bargain")));
    auto* Save=Mode->CaptureSnapshot();if(!TestNotNull(TEXT("Real snapshot includes completed graph flags"),Save))return false;
    TArray<uint8> Bytes;TestTrue(TEXT("Real save serializer accepts graph-created durable flags"),UGameplayStatics::SaveGameToMemory(Save,Bytes));
    auto* Loaded=Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
    if(!TestNotNull(TEXT("Real save deserializer"),Loaded))return false;
    TestTrue(TEXT("Lore memory survives serialization"),Loaded->SavedDialogueFlags.Contains(FName(TEXT("TORBEN_ROAD_HISTORY"))));
    TestTrue(TEXT("Canonical mission choice survives serialization"),Loaded->SavedDialogueFlags.Contains(FName(TEXT("SQ_001_CHOICE_bargain"))));
    TestTrue(TEXT("Authoritative load closes transient conversation"),Mode->RestoreSnapshot(Loaded,false));
    TestFalse(TEXT("No stale runtime cursor survives restore"),Fixture.Conversation->IsActive());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirConversationRestoredReturnTest,"Askir.Dialogue.Conversation.RestoredReturnResolvesTransientRewardData",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirConversationRestoredReturnTest::RunTest(const FString& Parameters)
{
    FConversationFixture Fixture;if(!TestTrue(TEXT("Controlled player owns conversation backend"),Fixture.Ready()))return false;
    UAskirItemData* Reward=Fixture.Reward();auto* Torben=Fixture.Torben();auto* Mode=Fixture.Spawn<AAskirGameMode>();
    if(!TestNotNull(TEXT("Authoritative game mode"),Mode))return false;
    auto* Delivery=Fixture.Player->DeliveryMission.Get();
    if(!TestTrue(TEXT("Prepare canonical mission"),Delivery->AcceptDelivery(TEXT("help"))))return false;
    TestTrue(TEXT("First bandit receipt"),Delivery->CreditBandit(TEXT("RESTORE_A")));
    TestTrue(TEXT("Second bandit receipt"),Delivery->CreditBandit(TEXT("RESTORE_B")));
    TestTrue(TEXT("Cargo receipt"),Delivery->RecoverCargo(TEXT("RESTORE_CARGO")));
    auto* Save=Mode->CaptureSnapshot();if(!TestNotNull(TEXT("Snapshot at return stage"),Save))return false;
    TArray<uint8> Bytes;
    if(!TestTrue(TEXT("Serialize return-stage snapshot"),UGameplayStatics::SaveGameToMemory(Save,Bytes)))return false;
    auto* Loaded=Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
    if(!TestNotNull(TEXT("Deserialize return-stage snapshot"),Loaded))return false;
    // Travel constructs a new player whose transient QuestData was never
    // populated by acceptance. Reproduce that boundary before authoritative load.
    Delivery->QuestData=nullptr;
    if(!TestTrue(TEXT("Restore durable mission ledger"),Mode->RestoreSnapshot(Loaded,false)))return false;
    TestNull(TEXT("Restore does not serialize the transient quest object"),Delivery->QuestData.Get());
    TestEqual(TEXT("Ledger restores the return stage"),Delivery->GetStage(),EAskirDeliveryStage::ReturnToTorben);
    if(!TestTrue(TEXT("Restored return dialogue opens"),Fixture.Conversation->BeginConversation(Torben,TEXT("D_SQ_001"))))return false;
    Fixture.Conversation->GetAvailableChoices();
    auto* Data=Delivery->QuestData.Get();
    if(!TestNotNull(TEXT("Dialogue readiness reconstructs canonical data before gating turn-in"),Data))return false;
    TestEqual(TEXT("Resolved canonical quest ID"),Data->QuestID,FName(TEXT("SQ_001")));
    TestEqual(TEXT("Resolved canonical gold"),Data->GoldReward,16);
    TestEqual(TEXT("Resolved canonical experience"),Data->ExperienceReward,100);
    // No generated asset is required for this test. A missing real reward must
    // remain disabled until a valid canonical item becomes available.
    if(!Data->RewardItem)
    {
        bool bDisabled=false;
        for(const auto& Choice:Fixture.Conversation->GetAvailableChoices())if(Choice.ChoiceID==FName(TEXT("turn_in")))bDisabled=!Choice.bEnabled;
        TestTrue(TEXT("Missing real reward safely disables turn-in"),bDisabled);
        Data->RewardItem=Reward;
    }
    else Reward=Data->RewardItem.Get();
    TestEqual(TEXT("Repeated canonical resolution preserves an existing valid reward"),Delivery->ResolveQuestData()->RewardItem.Get(),Reward);
    bool bEnabled=false;
    for(const auto& Choice:Fixture.Conversation->GetAvailableChoices())if(Choice.ChoiceID==FName(TEXT("turn_in")))bEnabled=Choice.bEnabled;
    TestTrue(TEXT("Valid reward enables the same restored conversation"),bEnabled);
    const int64 Revision=Fixture.Conversation->GetRevision();
    TestTrue(TEXT("Restored graph completes through real domain transaction"),Fixture.Conversation->SelectChoice(TEXT("turn_in"),Revision));
    TestEqual(TEXT("Exact gold paid once"),Fixture.Player->TradeComponent->Gold,16);
    TestEqual(TEXT("Exact reward item paid once"),Fixture.Player->InventoryComponent->GetItemCount(Reward),1);
    TestFalse(TEXT("Old restored turn-in token cannot pay twice"),Fixture.Conversation->SelectChoice(TEXT("turn_in"),Revision));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirConversationContextTest,"Askir.Dialogue.Conversation.SpeakerRangeHostilityCoverAndUnsupportedActions",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirConversationContextTest::RunTest(const FString& Parameters)
{
    FConversationFixture Fixture;if(!TestTrue(TEXT("Controlled player owns conversation backend"),Fixture.Ready()))return false;
    auto* Resident=Fixture.Resident();Resident->ConversationID=TEXT("D_SQ_001");
    TestTrue(TEXT("Friendly NPC can expose a catalog narrative"),Fixture.Conversation->BeginConversation(Resident));
    TestTrue(TEXT("Narrative can reach action node"),Fixture.Conversation->SelectChoice(TEXT("more"),Fixture.Conversation->GetRevision()));
    bool bTorbenActionDisabled=false;for(const auto& Choice:Fixture.Conversation->GetAvailableChoices())
        if(Choice.ChoiceID==FName(TEXT("help")))bTorbenActionDisabled=!Choice.bEnabled;
    TestTrue(TEXT("Assigning Torben's graph does not authorize another NPC's mission actions"),bTorbenActionDisabled);
    TestFalse(TEXT("Only a real delivery giver can execute acceptance"),Fixture.Conversation->SelectChoice(TEXT("help"),Fixture.Conversation->GetRevision()));
    Fixture.Conversation->EndConversation();Resident->ConversationID=TEXT("D_SQ_002");
    TestTrue(TEXT("NPC default catalog ID resolves when caller omits an ID"),Fixture.Conversation->BeginConversation(Resident));
    TestTrue(TEXT("Existing raw graph can present its narrative"),Fixture.Conversation->SelectChoice(TEXT("n1_choice_1"),Fixture.Conversation->GetRevision()));
    bool bDisabled=false;for(const auto& Choice:Fixture.Conversation->GetAvailableChoices())
        if(Choice.ChoiceID==FName(TEXT("n2_choice_1")))bDisabled=!Choice.bEnabled;
    TestTrue(TEXT("Unimplemented sidequest is explicitly disabled"),bDisabled);
    TestFalse(TEXT("Direct call cannot bypass unsupported quest action"),Fixture.Conversation->SelectChoice(TEXT("n2_choice_1"),Fixture.Conversation->GetRevision()));
    TestEqual(TEXT("Unsupported quest remains inactive"),Fixture.Player->QuestComponent->GetStateByID(TEXT("SQ_002")),EAskirQuestState::Inactive);
    Resident->SetActorLocation(FVector(600.,0.,96.));
    TestFalse(TEXT("Range is revalidated before any button action"),Fixture.Conversation->CanContinueConversation());
    Fixture.Conversation->TickComponent(.1f,LEVELTICK_All,nullptr);
    TestFalse(TEXT("Backend tick closes invalid speaker"),Fixture.Conversation->IsActive());
    Resident->SetActorLocation(FVector(180.,0.,96.));Resident->bHostile=true;
    TestFalse(TEXT("Hostile NPC cannot begin a friendly conversation"),Fixture.Conversation->BeginConversation(Resident));
    Resident->bHostile=false;Resident->SetEncounterActive(false);
    TestFalse(TEXT("Inactive NPC cannot begin a conversation"),Fixture.Conversation->BeginConversation(Resident));
    Resident->SetEncounterActive(true);
    AActor* Wall=Fixture.Spawn<AActor>(FVector(90.,0.,96.));auto* Box=Fixture.AddComponent<UBoxComponent>(Wall);Wall->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(15.,100.,150.));Box->SetCollisionProfileName(TEXT("BlockAll"));Wall->SetActorLocation(FVector(90.,0.,96.));
    TestFalse(TEXT("Blocking wall prevents conversation acquisition"),Fixture.Conversation->BeginConversation(Resident));
    Wall->SetActorEnableCollision(false);
    TestTrue(TEXT("Cleared cover permits conversation"),Fixture.Conversation->BeginConversation(Resident));
    Resident->Destroy();
    TestFalse(TEXT("Destroyed speaker invalidates immediately"),Fixture.Conversation->CanContinueConversation());
    Fixture.Conversation->TickComponent(.1f,LEVELTICK_All,nullptr);
    TestFalse(TEXT("Destroyed speaker releases runtime cursor"),Fixture.Conversation->IsActive());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirConversationFailedActionTest,"Askir.Dialogue.Conversation.ReturnCapacityFailureKeepsNodeAndNoReward",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirConversationFailedActionTest::RunTest(const FString& Parameters)
{
    FConversationFixture Fixture;if(!TestTrue(TEXT("Controlled player owns conversation backend"),Fixture.Ready()))return false;
    UAskirItemData* Reward=Fixture.Reward();auto* Torben=Fixture.Torben();
    TestTrue(TEXT("Prepare real canonical quest"),Fixture.Player->DeliveryMission->AcceptDelivery(TEXT("help")));
    Fixture.Player->DeliveryMission->CreditBandit(TEXT("FULL_A"));Fixture.Player->DeliveryMission->CreditBandit(TEXT("FULL_B"));Fixture.Player->DeliveryMission->RecoverCargo(TEXT("FULL_CARGO"));
    auto* Filler=NewObject<UAskirItemData>();Filler->ItemID=TEXT("V83_BLOCKING_ITEM");Fixture.Player->InventoryComponent->MaxInventorySlots=1;
    TestTrue(TEXT("Fill only inventory slot"),Fixture.Player->InventoryComponent->AddItem(Filler,1));
    TestTrue(TEXT("Return dialogue may open with full inventory"),Fixture.Conversation->BeginConversation(Torben,TEXT("D_SQ_001")));
    bool bDisabled=false;for(const auto& Choice:Fixture.Conversation->GetAvailableChoices())if(Choice.ChoiceID==FName(TEXT("turn_in")))bDisabled=!Choice.bEnabled;
    TestTrue(TEXT("Unavailable reward capacity disables completion"),bDisabled);
    TestFalse(TEXT("Direct call cannot bypass full inventory"),Fixture.Conversation->SelectChoice(TEXT("turn_in"),Fixture.Conversation->GetRevision()));
    TestEqual(TEXT("Rejected action keeps original node"),Fixture.Conversation->GetCurrentNode().NodeID,FName(TEXT("return")));
    TestEqual(TEXT("No partial reward gold"),Fixture.Player->TradeComponent->Gold,0);
    TestEqual(TEXT("No partial reward item"),Fixture.Player->InventoryComponent->GetItemCount(Reward),0);
    TestTrue(TEXT("Free inventory slot"),Fixture.Player->InventoryComponent->RemoveItem(Filler,1));
    TestTrue(TEXT("Same live conversation can retry after capacity recovers"),Fixture.Conversation->SelectChoice(TEXT("turn_in"),Fixture.Conversation->GetRevision()));
    TestEqual(TEXT("Successful retry grants exact item once"),Fixture.Player->InventoryComponent->GetItemCount(Reward),1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirConversationCallbackTest,"Askir.Dialogue.Conversation.RealDomainCallbackRejectsReentryAndHonorsCloseOrDestroy",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirConversationCallbackTest::RunTest(const FString& Parameters)
{
    // Each scenario receives a fresh world and real acceptance transaction. The
    // listener runs inside the actual reputation delegate, not a simulated guard.
    for(int Scenario=0;Scenario<3;++Scenario)
    {
        FConversationFixture Fixture;if(!TestTrue(TEXT("Controlled player owns conversation backend"),Fixture.Ready()))return false;
        Fixture.Reward();auto* Torben=Fixture.Torben();auto* Mode=Fixture.Spawn<AAskirGameMode>();
        auto* Probe=NewObject<UAskirConversationProbe>();Probe->Conversation=Fixture.Conversation;Probe->Player=Fixture.Player;
        Probe->Speaker=Torben;Probe->Mode=Mode;Probe->Snapshot=Mode->CaptureSnapshot();
        if(!TestNotNull(TEXT("Pre-action authoritative snapshot"),Probe->Snapshot.Get()))return false;
        Probe->bRequestClose=Scenario==0;Probe->bDestroySpeaker=Scenario==1;Probe->bDestroyPlayer=Scenario==2;
        auto* Reputation=Fixture.Player->ReputationComponent;
        Reputation->OnReputationChanged.AddDynamic(Probe,&UAskirConversationProbe::ObserveReputationChange);
        TestTrue(TEXT("Torben begins before callback scenario"),Fixture.Conversation->BeginConversation(Torben,TEXT("D_SQ_001")));
        TestTrue(TEXT("Action node reached before callback"),Fixture.Conversation->SelectChoice(TEXT("more"),Fixture.Conversation->GetRevision()));
        TestTrue(TEXT("Domain receipt commits even when callback closes or destroys context"),Fixture.Conversation->SelectChoice(TEXT("help"),Fixture.Conversation->GetRevision()));
        TestTrue(TEXT("Actual faction delegate executed"),Probe->bObserved);
        TestTrue(TEXT("Conversation mutation guard visible inside delegate"),Probe->bObservedTransaction);
        TestTrue(TEXT("Canonical receipt already installed before callback"),Probe->bObservedDomainReceipt);
        TestFalse(TEXT("Reentrant same-token choice is rejected"),Probe->bReentrantChoiceAccepted);
        TestFalse(TEXT("Reentrant begin cannot replace graph during action"),Probe->bReentrantBeginAccepted);
        TestTrue(TEXT("Callback cannot capture partial conversation/domain state"),Probe->bCaptureBlocked);
        TestTrue(TEXT("Callback cannot rewind its own domain receipt"),Probe->bRestoreBlocked);
        // UObject destruction is deferred until after this synchronous call. The
        // cursor flag remains observable without dereferencing a destroyed actor.
        TestFalse(TEXT("Deferred close or lost context cannot retain an active cursor"),Fixture.Conversation->IsActive());
        TestFalse(TEXT("Transaction guard is released on every callback outcome"),Fixture.Conversation->IsTransactionInProgress());
        if(Scenario!=2)Reputation->OnReputationChanged.RemoveDynamic(Probe,&UAskirConversationProbe::ObserveReputationChange);
    }
    return true;
}
#endif
