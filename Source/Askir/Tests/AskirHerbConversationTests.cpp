#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Dialogue/AskirConversationComponent.h"
#include "Mission/AskirHerbMissionComponent.h"
#include "NPC/AskirDialogueComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Trade/AskirTradeComponent.h"
#include "Social/AskirReputationComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "UObject/Package.h"

namespace
{
struct FHerbConversationFixture : FAskirAutomationTestWorld
{
    AAskirCharacter* Player=nullptr;
    APlayerController* Controller=nullptr;
    AAskirNPC* Rurik=nullptr;
    UAskirConversationComponent* Conversation=nullptr;
    UAskirHerbMissionComponent* Herbs=nullptr;
    FHerbConversationFixture()
    {
        if(!World)return;
        Player=Spawn<AAskirCharacter>(FVector(0,0,96));Controller=Spawn<APlayerController>();Rurik=Spawn<AAskirNPC>(FVector(180,0,96));
        if(!Player||!Controller||!Rurik)return;
        Controller->Possess(Player);Controller->SetControlRotation(FRotator::ZeroRotator);
        Player->GetCharacterMovement()->DisableMovement();Rurik->GetCharacterMovement()->DisableMovement();
        Rurik->bHostile=false;Rurik->ConversationID=TEXT("D_DORNFELD_RURIK");Rurik->Tags.Add(TEXT("ASKIR_V84_RURIK_GIVER"));
        Conversation=Player->FindComponentByClass<UAskirConversationComponent>();Herbs=Player->FindComponentByClass<UAskirHerbMissionComponent>();
    }
    bool Ready() const {return World&&Player&&Controller&&Rurik&&Conversation&&Herbs;}
    UAskirItemData* Reward()
    {
        auto* Package=CreatePackage(*(FString(TEXT("/Game/Askir/Automation/HerbConversation_"))+FGuid::NewGuid().ToString(EGuidFormats::Digits)));
        auto* Item=NewObject<UAskirItemData>(Package,TEXT("item_004"),RF_Public);Item->ItemID=TEXT("item_004");Item->MaxStackSize=99;
        auto* Data=NewObject<UAskirQuestData>(Player);Data->QuestID=TEXT("SQ_003");Data->GoldReward=24;Data->ExperienceReward=150;
        Data->RewardItem=Item;Data->RewardItemQuantity=1;
        for(FName ID:{FName(TEXT("SQ_003_A")),FName(TEXT("SQ_003_B"))})
        {FAskirQuestObjective Objective;Objective.ObjectiveID=ID;Objective.RequiredCount=1;Data->Objectives.Add(Objective);}
        Herbs->QuestData=Data;Player->InventoryComponent->Items.Reset();Player->TradeComponent->Gold=0;return Item;
    }
    bool Choose(FName ID){return Conversation->SelectChoice(ID,Conversation->GetRevision());}
    bool Enabled(FName ID) const
    {
        for(const auto& Choice:Conversation->GetAvailableChoices())if(Choice.ChoiceID==ID)return Choice.bEnabled;
        return false;
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirHerbConversationGiverTest,
    "Askir.Dialogue.V84.HerbActionsRequireTrustedRurikRatherThanAssignedGraph",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirHerbConversationGiverTest::RunTest(const FString& Parameters)
{
    FHerbConversationFixture Fixture;if(!TestTrue(TEXT("Player owns default herb and conversation components"),Fixture.Ready()))return false;
    Fixture.Reward();Fixture.Rurik->Tags.Remove(TEXT("ASKIR_V84_RURIK_GIVER"));
    if(!TestTrue(TEXT("Rurik lore remains available without quest authorization"),Fixture.Conversation->BeginConversation(Fixture.Rurik)))return false;
    TestTrue(TEXT("Lore graph can discuss the meadow"),Fixture.Choose(TEXT("herbs_available")));
    TestFalse(TEXT("Default graph alone cannot authorize a mission effect"),Fixture.Enabled(TEXT("help")));
    TestFalse(TEXT("Direct button call cannot bypass the missing giver marker"),Fixture.Choose(TEXT("help")));
    TestEqual(TEXT("Mission is untouched by unauthorized dialogue"),Fixture.Herbs->GetStage(),EAskirHerbStage::NotStarted);
    Fixture.Conversation->EndConversation();Fixture.Rurik->Tags.Add(TEXT("ASKIR_V84_RURIK_GIVER"));Fixture.Rurik->ConversationID=TEXT("D_SQ_003");
    if(!TestTrue(TEXT("An alternate narrative graph can still be displayed"),Fixture.Conversation->BeginConversation(Fixture.Rurik)))return false;
    TestTrue(TEXT("Alternate graph reaches its authored action node"),Fixture.Choose(TEXT("more")));
    TestFalse(TEXT("Marker alone does not authorize a mismatched NPC identity"),Fixture.Enabled(TEXT("help")));
    TestFalse(TEXT("Mismatched default identity cannot accept the quest"),Fixture.Choose(TEXT("help")));
    Fixture.Conversation->EndConversation();Fixture.Rurik->ConversationID=TEXT("D_DORNFELD_RURIK");
    if(!TestTrue(TEXT("Authorized existing Rurik identity opens lore"),Fixture.Conversation->BeginConversation(Fixture.Rurik)))return false;
    TestTrue(TEXT("Authorized Rurik reaches offer"),Fixture.Choose(TEXT("herbs_available")));
    TestTrue(TEXT("Authorized real native mission acceptance"),Fixture.Choose(TEXT("help")));
    TestEqual(TEXT("Canonical mission advances"),Fixture.Herbs->GetStage(),EAskirHerbStage::ExploreMeadow);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirHerbConversationVirtualStageTest,
    "Askir.Dialogue.V84.CachedStageFlagsCannotForgeAuthoritativeQuestEntry",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirHerbConversationVirtualStageTest::RunTest(const FString& Parameters)
{
    FHerbConversationFixture Fixture;if(!TestTrue(TEXT("Default components"),Fixture.Ready()))return false;
    Fixture.Reward();Fixture.Player->DialogueComponent->AddFlag(TEXT("SQ_003_STAGE_RETURN"));
    Fixture.Player->DialogueComponent->AddFlag(TEXT("sq_003_stage_COMPLETED"));
    const auto Before=Fixture.Player->DialogueComponent->GetFlags();
    if(!TestTrue(TEXT("Actual ledger-backed graph opens"),Fixture.Conversation->BeginConversation(Fixture.Rurik,TEXT("D_SQ_003"))))return false;
    TestEqual(TEXT("Forged stored stage flags cannot select return or completion"),Fixture.Conversation->GetCurrentNode().NodeID,FName(TEXT("n1")));
    TestFalse(TEXT("No forged return choice is available"),Fixture.Enabled(TEXT("turn_in")));
    TestTrue(TEXT("Pure entry/choice evaluation does not persist virtual stage keys"),Before==Fixture.Player->DialogueComponent->GetFlags());
    TestTrue(TEXT("Real stage still offers canonical acceptance"),Fixture.Choose(TEXT("more")));
    TestTrue(TEXT("A valid choice accepts canonical ledger state"),Fixture.Choose(TEXT("bargain")));
    Fixture.Conversation->EndConversation();
    if(!TestTrue(TEXT("Reopen after real acceptance"),Fixture.Conversation->BeginConversation(Fixture.Rurik,TEXT("D_SQ_003"))))return false;
    TestEqual(TEXT("Real exploration stage wins over stored counterfeit flags"),Fixture.Conversation->GetCurrentNode().NodeID,FName(TEXT("active")));
    TestFalse(TEXT("Derived exploration flag is not saved as a second authority"),Fixture.Player->DialogueComponent->HasFlag(TEXT("SQ_003_STAGE_EXPLORE")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirHerbConversationFlowTest,
    "Askir.Dialogue.V84.PreservedRurikLoreHerbStagesCapacityRetryAndSingleReward",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirHerbConversationFlowTest::RunTest(const FString& Parameters)
{
    FHerbConversationFixture Fixture;if(!TestTrue(TEXT("Default components"),Fixture.Ready()))return false;
    auto* Reward=Fixture.Reward();
    if(!TestTrue(TEXT("Existing Rurik graph opens"),Fixture.Conversation->BeginConversation(Fixture.Rurik)))return false;
    TestTrue(TEXT("Existing well lore branch remains callable"),Fixture.Choose(TEXT("well")));
    Fixture.Conversation->EndConversation();
    TestTrue(TEXT("Remembered Rurik opens"),Fixture.Conversation->BeginConversation(Fixture.Rurik));
    TestEqual(TEXT("Well knowledge keeps its V83 greeting"),Fixture.Conversation->GetCurrentNode().NodeID,FName(TEXT("remembered")));
    TestTrue(TEXT("Existing greeting leads to new quest offer"),Fixture.Choose(TEXT("herbs_available")));
    TestTrue(TEXT("Help choice accepts native mission"),Fixture.Choose(TEXT("help")));
    Fixture.Conversation->EndConversation();
    TestTrue(TEXT("Active mission dialogue opens"),Fixture.Conversation->BeginConversation(Fixture.Rurik));
    TestTrue(TEXT("Active exploration branch is visible"),Fixture.Enabled(TEXT("herbs_active")));
    TestFalse(TEXT("Return branch stays absent before exploring"),Fixture.Enabled(TEXT("herbs_return")));
    Fixture.Conversation->EndConversation();
    TestTrue(TEXT("Native exploration installs actual canonical receipt"),Fixture.Herbs->ExploreHerbs(UAskirHerbMissionComponent::GetCanonicalSourceID()));
    auto* Filler=NewObject<UAskirItemData>();Filler->ItemID=TEXT("V84_BLOCKING_ITEM");Fixture.Player->InventoryComponent->MaxInventorySlots=1;
    TestTrue(TEXT("Only inventory slot filled"),Fixture.Player->InventoryComponent->AddItem(Filler,1));
    TestTrue(TEXT("Return conversation opens"),Fixture.Conversation->BeginConversation(Fixture.Rurik));
    TestTrue(TEXT("Real exploration makes return branch available"),Fixture.Choose(TEXT("herbs_return")));
    TestFalse(TEXT("Full bag disables atomic reward action"),Fixture.Enabled(TEXT("turn_in")));
    TestFalse(TEXT("Direct turn-in cannot bypass a full bag"),Fixture.Choose(TEXT("turn_in")));
    TestEqual(TEXT("Failed return keeps retry node"),Fixture.Conversation->GetCurrentNode().NodeID,FName(TEXT("herbs_return")));
    TestEqual(TEXT("No partial gold payout"),Fixture.Player->TradeComponent->Gold,0);
    TestTrue(TEXT("Inventory space released"),Fixture.Player->InventoryComponent->RemoveItem(Filler,1));
    const int64 Token=Fixture.Conversation->GetRevision();
    TestTrue(TEXT("Same session retries actual mission transaction"),Fixture.Choose(TEXT("turn_in")));
    TestEqual(TEXT("Canonical 24 gold"),Fixture.Player->TradeComponent->Gold,24);
    TestEqual(TEXT("Exactly one canonical material item"),Fixture.Player->InventoryComponent->GetItemCount(Reward),1);
    TestEqual(TEXT("Help choice affects Waldhüter once"),Fixture.Player->ReputationComponent->GetReputation(TEXT("Waldhüter")),2);
    TestFalse(TEXT("Old return button cannot pay again"),Fixture.Conversation->SelectChoice(TEXT("turn_in"),Token));
    Fixture.Conversation->EndConversation();
    TestTrue(TEXT("Completed Rurik discussion opens"),Fixture.Conversation->BeginConversation(Fixture.Rurik));
    TestTrue(TEXT("Completion branch replaces the quest offer"),Fixture.Enabled(TEXT("herbs_completed")));
    TestFalse(TEXT("Quest cannot be offered again"),Fixture.Enabled(TEXT("herbs_available")));
    TestTrue(TEXT("Existing well lore survived the complete new quest"),Fixture.Player->DialogueComponent->HasFlag(TEXT("RURIK_WELL_LORE")));
    for(FName Flag:Fixture.Player->DialogueComponent->GetFlags())
        TestFalse(TEXT("No virtual stage was persisted"),Flag.ToString().StartsWith(TEXT("SQ_003_STAGE_"),ESearchCase::IgnoreCase));
    return true;
}
#endif
