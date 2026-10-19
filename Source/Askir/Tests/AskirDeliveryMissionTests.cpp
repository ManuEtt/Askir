#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Tests/AskirTransactionPersistenceProbe.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Combat/AskirCombatComponent.h"
#include "Game/AskirGameMode.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Mission/AskirDeliveryBanditRelay.h"
#include "Mission/AskirDeliveryMissionComponent.h"
#include "Mission/AskirDeliveryQuestActor.h"
#include "NPC/AskirDialogueComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Quest/AskirQuestData.h"
#include "Save/AskirSaveGame.h"
#include "Social/AskirReputationComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Package.h"
#include <limits>

namespace
{
UAskirItemData* DeliveryRewardAsset()
{
    // A resident /Game fixture can serialize its soft reference. This creates no
    // disk asset and makes no claim that the generated village map was imported.
    UPackage* Package=CreatePackage(*(FString(TEXT("/Game/Askir/Automation/Delivery_"))+FGuid::NewGuid().ToString(EGuidFormats::Digits)));
    UAskirItemData* Item=NewObject<UAskirItemData>(Package,TEXT("item_002"),RF_Public);
    Item->ItemID=TEXT("item_002");Item->MaxStackSize=99;return Item;
}
UAskirQuestData* DeliveryQuestData(UObject* Outer,UAskirItemData* Item)
{
    UAskirQuestData* Quest=NewObject<UAskirQuestData>(Outer);
    Quest->QuestID=TEXT("SQ_001");Quest->GoldReward=16;Quest->ExperienceReward=100;
    Quest->RewardItem=Item;Quest->RewardItemQuantity=1;
    const FName IDs[]={TEXT("SQ_001_A"),TEXT("SQ_001_CARGO"),TEXT("SQ_001_B")};
    for(int32 Index=0;Index<3;++Index)
    {
        FAskirQuestObjective Objective;Objective.ObjectiveID=IDs[Index];Objective.RequiredCount=Index==0?2:1;
        Quest->Objectives.Add(Objective);
    }
    return Quest;
}
int64 TotalXP(const FAskirProgressionSnapshot& Progress)
{
    int64 Result=Progress.Experience;
    for(int64 Level=1;Level<Progress.Level;++Level)Result+=100*Level*Level;
    return Result;
}
AAskirCharacter* DeliveryPlayer(FAskirAutomationTestWorld& Fixture,UAskirItemData* Reward)
{
    AAskirCharacter* Player=Fixture.Spawn<AAskirCharacter>(FVector(0.f,0.f,96.f));
    APlayerController* Controller=Fixture.Spawn<APlayerController>();
    if(!Player || !Controller)return nullptr;
    Controller->Possess(Player);
    Player->InventoryComponent->Items.Reset();Player->TradeComponent->Gold=0;
    Player->ProgressionComponent->RestoreProgress(FAskirProgressionSnapshot());
    Player->DeliveryMission->QuestData=DeliveryQuestData(Player,Reward);
    return Player;
}
void Hit(AAskirNPC* NPC,AActor* Causer,float Amount=1000.f)
{
    FAskirDamageInfo Damage;Damage.Damage=Amount;Damage.DamageCauser=Causer;
    NPC->CombatComponent->ApplyDamage(Damage);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirDeliveryCombatSaveTest,"Askir.Mission.Delivery.AttributedBanditsCargoRewardAndEarlierLoad",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirDeliveryCombatSaveTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Runtime world"),Fixture.World))return false;
    UAskirItemData* Reward=DeliveryRewardAsset();
    AAskirCharacter* Player=DeliveryPlayer(Fixture,Reward);
    if(!TestNotNull(TEXT("Possessed quest player"),Player))return false;
    AAskirGameMode* Mode=Fixture.Spawn<AAskirGameMode>();
    UAskirDeliveryMissionComponent* Mission=Player->DeliveryMission;
    AAskirDeliveryQuestActor* Cargo=Fixture.Spawn<AAskirDeliveryQuestActor>(FVector(0.f,150.f,96.f));
    Cargo->Role=EAskirDeliveryQuestRole::Cargo;Cargo->EventID=TEXT("SQ_001_CARGO");Cargo->MissionPlayer=Player;
    Cargo->BindMissionEvents();
    AAskirNPC* Bandits[2];AAskirDeliveryBanditRelay* Relays[2];
    for(int32 Index=0;Index<2;++Index)
    {
        Bandits[Index]=Fixture.Spawn<AAskirNPC>(FVector(5000.f+Index*1000.f,0.f,96.f));
        Bandits[Index]->KillExperience=90;Bandits[Index]->GuaranteedDrop=Reward;
        Relays[Index]=Fixture.Spawn<AAskirDeliveryBanditRelay>();
        Relays[Index]->BanditActor=Bandits[Index];Relays[Index]->MissionPlayer=Player;
        Relays[Index]->EventID=Index==0?FName(TEXT("SQ_001_BANDIT_1")):FName(TEXT("SQ_001_BANDIT_2"));
        TestTrue(TEXT("Native death relay binds"),Relays[Index]->BindMissionEvents());
        TestEqual(TEXT("Relay disables early per-kill experience"),Bandits[Index]->KillExperience,0);
        TestNull(TEXT("Relay disables early item drop"),Bandits[Index]->GuaranteedDrop.Get());
        TestTrue(TEXT("No global objective event can double-advance this bandit"),Bandits[Index]->DefeatObjectiveID.IsNone());
    }
    TestFalse(TEXT("Bandits cannot be damaged before accepting Torben's request"),Bandits[0]->CombatComponent->IsDamageEnabled());
    TestFalse(TEXT("Approaching cargo before acceptance grants nothing"),IAskirInteractable::Execute_Interact(Cargo,Player));
    TestFalse(TEXT("Cargo interaction does not implicitly accept the quest"),Player->QuestComponent->GetStateByID(TEXT("SQ_001"))==EAskirQuestState::Active);
    TestTrue(TEXT("Torben help choice accepts once"),Mission->AcceptDelivery(TEXT("help")));
    TestFalse(TEXT("Second choice cannot grant another faction reward"),Mission->AcceptDelivery(TEXT("bargain")));
    TestEqual(TEXT("Help choice grants Waldhüter +2"),Player->ReputationComponent->GetReputation(TEXT("Waldhüter")),2);
    TestEqual(TEXT("Unchosen bargain grants no reputation"),Player->ReputationComponent->GetReputation(TEXT("Freie Händler")),0);
    TestEqual(TEXT("Accepted choice comes from dialogue flags"),Mission->GetAcceptedChoice(),FName(TEXT("help")));
    TestTrue(TEXT("Accepted quest activates hostile bandits"),Bandits[0]->IsEncounterActive());
    TestFalse(TEXT("Cargo remains gated by both actual bandit deaths"),IAskirInteractable::Execute_Interact(Cargo,Player));
    Hit(Bandits[1],Player,10.f);
    const float WoundedHealth=Bandits[1]->CombatComponent->GetHealth();
    Hit(Bandits[0],Player);
    TestEqual(TEXT("First attributed player death is persisted"),Mission->GetBanditCount(),1);
    TestEqual(TEXT("Progress does not heal the other wounded bandit"),Bandits[1]->CombatComponent->GetHealth(),WoundedHealth);
    TestEqual(TEXT("Bandit cannot pay a reward before Torben"),TotalXP(Player->ProgressionComponent->Progress),int64(0));
    TestEqual(TEXT("Bandit cannot drop the final reward item"),Player->InventoryComponent->GetItemCount(Reward),0);
    UAskirSaveGame* Halfway=Mode->CaptureSnapshot();
    if(!TestNotNull(TEXT("Full generic save after one credited kill"),Halfway))return false;
    FString Reason;
    if(!TestTrue(TEXT("Halfway quest receipts and objective mirrors validate"),Halfway->ValidateSnapshot(Reason)))return false;
    TArray<uint8> Bytes;
    if(!TestTrue(TEXT("Serialize full halfway SaveGame"),UGameplayStatics::SaveGameToMemory(Halfway,Bytes)))return false;

    Bandits[1]->CombatComponent->ClearDamageAttribution();Bandits[1]->CombatComponent->Kill();
    TestEqual(TEXT("Environmental death never credits a player kill"),Mission->GetBanditCount(),1);
    Fixture.World->Tick(LEVELTICK_All,.01f);
    TestFalse(TEXT("Uncredited required bandit is restored next tick"),Bandits[1]->IsDead());
    AAskirNPC* OtherNPC=Fixture.Spawn<AAskirNPC>(FVector(10000.f,0.f,96.f));
    Hit(Bandits[1],OtherNPC);
    TestEqual(TEXT("Another NPC's killing blow cannot credit SQ_001"),Mission->GetBanditCount(),1);
    Fixture.World->Tick(LEVELTICK_All,.01f);
    TestFalse(TEXT("NPC kill restores the required bandit without a receipt"),Bandits[1]->IsDead());
    Hit(Bandits[1],Player);
    TestEqual(TEXT("Two distinct player kills unlock cargo"),Mission->GetStage(),EAskirDeliveryStage::RecoverCargo);
    TestFalse(TEXT("Duplicate bandit identity cannot count again"),Mission->CreditBandit(TEXT("SQ_001_BANDIT_1")));
    TestFalse(TEXT("Killing both bandits alone cannot finish the quest"),Mission->ReturnToTorben());
    TestTrue(TEXT("Physical cargo interaction records recovery"),IAskirInteractable::Execute_Interact(Cargo,Player));
    TestTrue(TEXT("Recovered cargo is hidden"),Cargo->IsHidden());
    TestFalse(TEXT("Recovered cargo cannot be collected twice"),IAskirInteractable::Execute_Interact(Cargo,Player));

    UAskirTransactionPersistenceProbe* Probe=NewObject<UAskirTransactionPersistenceProbe>();
    Probe->Mode=Mode;Probe->Snapshot=Halfway;
    Player->InventoryComponent->OnInventoryChanged.AddDynamic(Probe,&UAskirTransactionPersistenceProbe::ObserveInventoryChange);
    TestTrue(TEXT("Confirmed return pays the complete quest reward"),Mission->ReturnToTorben());
    TestTrue(TEXT("Reward inventory delegate executes"),Probe->bCallbackObserved);
    TestTrue(TEXT("Saving inside reward delegate is blocked"),Probe->bCaptureBlocked);
    TestTrue(TEXT("Loading inside reward delegate is blocked"),Probe->bRestoreBlocked);
    Player->InventoryComponent->OnInventoryChanged.RemoveDynamic(Probe,&UAskirTransactionPersistenceProbe::ObserveInventoryChange);
    TestEqual(TEXT("Torben grants exactly 16 gold"),Player->TradeComponent->Gold,16);
    TestEqual(TEXT("Torben grants exactly 100 total XP"),TotalXP(Player->ProgressionComponent->Progress),int64(100));
    TestEqual(TEXT("Torben grants canonical item_002 once"),Player->InventoryComponent->GetItemCount(Reward),1);
    TestTrue(TEXT("Generic claimed reward receipt exists"),Player->QuestComponent->HasClaimedReward(TEXT("SQ_001")));
    TestFalse(TEXT("Turn-in cannot duplicate reward"),Mission->ReturnToTorben());

    UAskirSaveGame* Loaded=Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
    if(!TestNotNull(TEXT("Deserialize halfway SaveGame"),Loaded))return false;
    if(!TestTrue(TEXT("Restore earlier generic timeline"),Mode->RestoreSnapshot(Loaded,false)))return false;
    TestEqual(TEXT("Earlier load restores one kill"),Mission->GetBanditCount(),1);
    TestTrue(TEXT("Credited bandit stays dead after rewind"),Bandits[0]->IsDead());
    TestFalse(TEXT("Uncredited bandit revives after rewind"),Bandits[1]->IsDead());
    TestFalse(TEXT("Cargo reappears after earlier load"),Cargo->IsHidden());
    TestEqual(TEXT("Earlier load removes outgoing reward item"),Player->InventoryComponent->GetItemCount(Reward),0);
    TestEqual(TEXT("Earlier load restores gold"),Player->TradeComponent->Gold,0);
    TestEqual(TEXT("Faction choice survives full restore once"),Player->ReputationComponent->GetReputation(TEXT("Waldhüter")),2);
    TestFalse(TEXT("Restored accepted choice cannot be changed or repeated"),Mission->AcceptDelivery(TEXT("help")));
    Hit(Bandits[1],Player);
    TestTrue(TEXT("Restored cargo can be secured normally"),IAskirInteractable::Execute_Interact(Cargo,Player));
    TestTrue(TEXT("Restored quest can finish normally"),Mission->ReturnToTorben());
    UAskirSaveGame* Completed=Mode->CaptureSnapshot();
    if(!TestNotNull(TEXT("Completed snapshot"),Completed))return false;
    TestTrue(TEXT("Completed snapshot restores"),Mode->RestoreSnapshot(Completed,false));
    TestTrue(TEXT("Completed snapshot can be restored repeatedly"),Mode->RestoreSnapshot(Completed,false));
    TestFalse(TEXT("Completed save forbids duplicate Torben payout"),Mission->ReturnToTorben());
    TestEqual(TEXT("Repeated load preserves one reward item"),Player->InventoryComponent->GetItemCount(Reward),1);
    TestEqual(TEXT("Repeated load preserves 16 gold"),Player->TradeComponent->Gold,16);
    TestEqual(TEXT("Repeated load preserves exactly 100 XP"),TotalXP(Player->ProgressionComponent->Progress),int64(100));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirDeliveryRewardPreflightTest,"Askir.Mission.Delivery.InvalidChoiceFullBagAndPayoutBoundary",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirDeliveryRewardPreflightTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Runtime world"),Fixture.World))return false;
    UAskirItemData* Reward=DeliveryRewardAsset();AAskirCharacter* Player=DeliveryPlayer(Fixture,Reward);
    if(!TestNotNull(TEXT("Quest player"),Player))return false;
    UAskirDeliveryMissionComponent* Mission=Player->DeliveryMission;
    TestFalse(TEXT("Unknown branch cannot accept or alter faction standing"),Mission->AcceptDelivery(TEXT("invented")));
    TestEqual(TEXT("Unknown branch leaves quest inactive"),Mission->GetStage(),EAskirDeliveryStage::NotStarted);
    TestTrue(TEXT("Bargain branch accepts"),Mission->AcceptDelivery(TEXT("bargain")));
    TestEqual(TEXT("Bargain branch records canonical dialogue flag"),Mission->GetAcceptedChoice(),FName(TEXT("bargain")));
    TestEqual(TEXT("Bargain grants Freie Händler +2"),Player->ReputationComponent->GetReputation(TEXT("Freie Händler")),2);
    TestFalse(TEXT("Cargo cannot bypass first bandit"),Mission->RecoverCargo(TEXT("SQ_001_CARGO")));
    TestTrue(TEXT("First unique receipt"),Mission->CreditBandit(TEXT("FIRST")));
    TestFalse(TEXT("Duplicate receipt rejected"),Mission->CreditBandit(TEXT("FIRST")));
    TestFalse(TEXT("Cargo cannot bypass second bandit"),Mission->RecoverCargo(TEXT("SQ_001_CARGO")));
    TestTrue(TEXT("Second unique receipt"),Mission->CreditBandit(TEXT("SECOND")));
    TestTrue(TEXT("Cargo receipt"),Mission->RecoverCargo(TEXT("SQ_001_CARGO")));
    Player->InventoryComponent->MaxInventorySlots=1;
    UAskirItemData* BlockingItem=NewObject<UAskirItemData>();BlockingItem->ItemID=TEXT("BlockingItem");
    TestTrue(TEXT("Fill the only available slot"),Player->InventoryComponent->AddItem(BlockingItem,1));
    TestFalse(TEXT("Full bag rejects the complete payout"),Mission->ReturnToTorben());
    TestEqual(TEXT("Failed reward leaves gold untouched"),Player->TradeComponent->Gold,0);
    TestEqual(TEXT("Failed reward leaves XP untouched"),TotalXP(Player->ProgressionComponent->Progress),int64(0));
    TestFalse(TEXT("Failed reward cannot claim the receipt"),Player->QuestComponent->HasClaimedReward(TEXT("SQ_001")));
    TestEqual(TEXT("Failed reward rolls back the return objective"),Player->QuestComponent->GetObjectivesForQuest(TEXT("SQ_001"))[2].CurrentCount,0);
    TestTrue(TEXT("Free the slot"),Player->InventoryComponent->RemoveItem(BlockingItem,1));
    Player->TradeComponent->Gold=MAX_int32-15;
    TestFalse(TEXT("Overflowing payout rejects before item or XP mutation"),Mission->ReturnToTorben());
    TestEqual(TEXT("Overflow failure leaves inventory empty"),Player->InventoryComponent->GetItemCount(Reward),0);
    TestEqual(TEXT("Overflow failure leaves the return objective untouched"),Player->QuestComponent->GetObjectivesForQuest(TEXT("SQ_001"))[2].CurrentCount,0);
    Player->TradeComponent->Gold=0;Player->ProgressionComponent->Progress.UnspentPoints=MAX_int32;
    TestFalse(TEXT("XP point overflow rejects the complete delivery payout"),Mission->ReturnToTorben());
    TestEqual(TEXT("XP failure rolls back the return objective"),Player->QuestComponent->GetObjectivesForQuest(TEXT("SQ_001"))[2].CurrentCount,0);
    TestEqual(TEXT("XP failure leaves the item reward untouched"),Player->InventoryComponent->GetItemCount(Reward),0);
    TestEqual(TEXT("XP failure leaves gold untouched"),Player->TradeComponent->Gold,0);
    Player->ProgressionComponent->Progress.UnspentPoints=0;
    Player->TradeComponent->Gold=MAX_int32-16;
    TestTrue(TEXT("Exact wallet boundary pays once"),Mission->ReturnToTorben());
    TestEqual(TEXT("Wallet reaches exactly MAX_int32"),Player->TradeComponent->Gold,MAX_int32);
    TestEqual(TEXT("One canonical reward item"),Player->InventoryComponent->GetItemCount(Reward),1);
    TestFalse(TEXT("Completed branch cannot change choice"),Mission->AcceptDelivery(TEXT("help")));
    TestEqual(TEXT("Bargain reputation is still granted once"),Player->ReputationComponent->GetReputation(TEXT("Freie Händler")),2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirDeliveryInteractionBoundsTest,"Askir.Mission.Delivery.InteractionDistanceAndDeadPlayer",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirDeliveryInteractionBoundsTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Runtime world"),Fixture.World))return false;
    AAskirCharacter* Player=DeliveryPlayer(Fixture,DeliveryRewardAsset());
    if(!TestNotNull(TEXT("Quest player"),Player))return false;
    AAskirDeliveryQuestActor* Cargo=Fixture.Spawn<AAskirDeliveryQuestActor>(FVector(0.f,150.f,96.f));
    Cargo->Role=EAskirDeliveryQuestRole::Cargo;Cargo->EventID=TEXT("SQ_001_CARGO");
    TestTrue(TEXT("Nearby living player can inspect the cargo"),Cargo->CanInteractWith(Player));
    Cargo->InteractionRange=std::numeric_limits<float>::quiet_NaN();
    TestFalse(TEXT("Nonfinite configured range cannot authorize interaction"),Cargo->CanInteractWith(Player));
    Cargo->InteractionRange=-1.f;TestFalse(TEXT("Negative range cannot authorize interaction"),Cargo->CanInteractWith(Player));
    Cargo->InteractionRange=250.f;Cargo->SetActorLocation(FVector(5000.f,0.f,96.f));
    TestFalse(TEXT("Remote actor cannot be activated by a stale dialogue"),Cargo->CanInteractWith(Player));
    Cargo->SetActorLocation(FVector(0.f,150.f,96.f));Player->CombatComponent->Kill();
    TestFalse(TEXT("Dead player cannot interact"),Cargo->CanInteractWith(Player));
    TestFalse(TEXT("Dead player cannot accept the mission directly"),Player->DeliveryMission->AcceptDelivery(TEXT("help")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirDeliveryLateBindAndDeadKillTest,"Askir.Mission.Delivery.LatePlayerBindingAndDeadPlayerAttribution",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirDeliveryLateBindAndDeadKillTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Runtime world"),Fixture.World))return false;
    AAskirNPC* Bandit=Fixture.Spawn<AAskirNPC>(FVector(1000.f,0.f,96.f));
    AAskirDeliveryBanditRelay* Relay=Fixture.Spawn<AAskirDeliveryBanditRelay>();
    if(!TestNotNull(TEXT("Placed bandit"),Bandit) || !TestNotNull(TEXT("Placed relay"),Relay))return false;
    Relay->BanditActor=Bandit;Relay->EventID=TEXT("LATE_BANDIT");
    TestFalse(TEXT("Relay waits while no player exists"),Relay->BindMissionEvents());
    AAskirCharacter* Player=DeliveryPlayer(Fixture,DeliveryRewardAsset());
    if(!TestNotNull(TEXT("Later possessed player"),Player))return false;
    Fixture.World->Tick(LEVELTICK_All,.6f);
    TestFalse(TEXT("Retry binds and gates the unaccepted bandit"),Bandit->CombatComponent->IsDamageEnabled());
    TestTrue(TEXT("Late-bound player accepts normally"),Player->DeliveryMission->AcceptDelivery(TEXT("help")));
    TestTrue(TEXT("Late-bound relay responds to quest activation"),Bandit->CombatComponent->IsDamageEnabled());
    Player->CombatComponent->Kill();
    Hit(Bandit,Player);
    TestEqual(TEXT("Dead player's outgoing damage cannot create a quest receipt"),Player->DeliveryMission->GetBanditCount(),0);
    Fixture.World->Tick(LEVELTICK_All,.01f);
    TestFalse(TEXT("Uncredited bandit remains available in the saved encounter"),Bandit->IsDead());
    auto* NPCOwner=Fixture.Spawn<AAskirNPC>();
    TestFalse(TEXT("Unpossessed NPC delivery component cannot accept a player quest"),NPCOwner->DeliveryMission->AcceptDelivery(TEXT("help")));
    return true;
}
#endif
