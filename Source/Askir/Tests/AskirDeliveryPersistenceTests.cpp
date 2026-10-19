#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Tests/AskirTransactionPersistenceProbe.h"
#include "Characters/AskirCharacter.h"
#include "Game/AskirGameMode.h"
#include "Mission/AskirDeliveryMissionComponent.h"
#include "NPC/AskirDialogueComponent.h"
#include "Social/AskirReputationComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Save/AskirSaveGame.h"
#include "Economy/AskirMerchantComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirDeliverySaveLedgerRoundTripTest,"Askir.Persistence.DeliveryChoiceReceiptsAndMerchantIsolation",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirDeliverySaveLedgerRoundTripTest::RunTest(const FString& Parameters)
{
 FAskirAutomationTestWorld Fixture;
 if(!TestNotNull(TEXT("Initialized runtime world"),Fixture.World))return false;
 auto* Player=Fixture.Spawn<AAskirCharacter>();auto* Controller=Fixture.Spawn<APlayerController>();
 auto* Mode=Fixture.Spawn<AAskirGameMode>();auto* Store=Fixture.Spawn<AActor>();
 if(!TestNotNull(TEXT("Player"),Player) || !TestNotNull(TEXT("Controller"),Controller)
  || !TestNotNull(TEXT("Save mode"),Mode) || !TestNotNull(TEXT("Store actor"),Store))return false;
 Controller->Possess(Player);
 auto* Dialogue=Player->FindComponentByClass<UAskirDialogueComponent>();
 auto* Reputation=Player->FindComponentByClass<UAskirReputationComponent>();
 if(!TestNotNull(TEXT("Player owns persisted dialogue component"),Dialogue)
  || !TestNotNull(TEXT("Player owns persisted reputation component"),Reputation))return false;
 auto* Merchant=NewObject<UAskirMerchantComponent>(Store);
 Merchant->MerchantID=TEXT("GENERAL_STORE");Merchant->MerchantGold=500;
 FAskirShopItem Stock;Stock.ItemID=TEXT("Ore");Stock.BasePrice=20;Stock.Stock=10;Stock.MaxStock=10;
 Merchant->Inventory.Add(Stock);Store->AddInstanceComponent(Merchant);Merchant->RegisterComponent();
 auto* AcceptanceProbe=NewObject<UAskirTransactionPersistenceProbe>();AcceptanceProbe->Mode=Mode;AcceptanceProbe->Snapshot=Mode->CaptureSnapshot();
 if(!TestNotNull(TEXT("Pre-acceptance snapshot"),AcceptanceProbe->Snapshot.Get()))return false;
 Reputation->OnReputationChanged.AddDynamic(AcceptanceProbe,&UAskirTransactionPersistenceProbe::ObserveReputationChange);
 TestTrue(TEXT("Actual delivery acceptance records bargain choice"),Player->DeliveryMission->AcceptDelivery(TEXT("bargain")));
 Reputation->OnReputationChanged.RemoveDynamic(AcceptanceProbe,&UAskirTransactionPersistenceProbe::ObserveReputationChange);
 TestTrue(TEXT("Real reputation callback observed during acceptance"),AcceptanceProbe->bCallbackObserved);
 TestTrue(TEXT("Acceptance callback cannot capture incomplete choices"),AcceptanceProbe->bCaptureBlocked);
 TestTrue(TEXT("Acceptance callback cannot rewind its own receipt"),AcceptanceProbe->bRestoreBlocked);
 TestTrue(TEXT("First stable encounter receipt"),Player->DeliveryMission->CreditBandit(TEXT("SAVE_A")));
 TestTrue(TEXT("Second stable encounter receipt"),Player->DeliveryMission->CreditBandit(TEXT("SAVE_B")));
 TestTrue(TEXT("Cargo receipt recorded after encounter"),Player->DeliveryMission->RecoverCargo(TEXT("SAVE_CARGO")));
 Player->TradeComponent->Gold=23;Merchant->MerchantGold=540;Merchant->Inventory[0].Stock=8;
 auto* Save=Mode->CaptureSnapshot();
 if(!TestNotNull(TEXT("Actual complete player snapshot"),Save))return false;
 FAskirMerchantSnapshot Remote;Remote.Gold=311;Remote.StockByItem.Add(TEXT("Ore"),3);
 Save->SavedMerchantsByMap.FindOrAdd(TEXT("AnotherRegion")).Merchants.Add(Merchant->MerchantID,Remote);
 FString Reason;
 if(!TestTrue(TEXT("Canonical quest snapshot validates"),Save->ValidateSnapshot(Reason)))return false;
 TArray<uint8> Bytes;
 if(!TestTrue(TEXT("Real SaveGame serializer preserves all ledgers"),UGameplayStatics::SaveGameToMemory(Save,Bytes)))return false;
 auto* Loaded=Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
 if(!TestNotNull(TEXT("Real SaveGame deserializer"),Loaded))return false;
 TestTrue(TEXT("Choice flag survives serialization"),Loaded->SavedDialogueFlags.Contains(FName(TEXT("SQ_001_CHOICE_bargain"))));
 TestTrue(TEXT("Acceptance receipt survives serialization"),Loaded->SavedConsumedQuestTriggers.Contains(FName(TEXT("SQ_001_ACCEPTED"))));
 TestTrue(TEXT("Unique encounter receipt survives serialization"),Loaded->SavedConsumedQuestTriggers.Contains(FName(TEXT("SQ_001_KILL_SAVE_A"))));
 TestEqual(TEXT("Same merchant ID remains isolated on another map"),Loaded->SavedMerchantsByMap[TEXT("AnotherRegion")].Merchants[Merchant->MerchantID].Gold,311);
 Dialogue->RestoreFlags({});Reputation->SetReputation(TEXT("Freie Händler"),-75);
 Player->QuestComponent->RestoreQuestProgress(TEXT("SQ_001"),{},EAskirQuestState::Inactive,false,false);
 Player->QuestComponent->ConsumedQuestTriggers.Reset();Player->TradeComponent->Gold=999;
 Merchant->MerchantGold=1;Merchant->Inventory[0].Stock=0;
 TestTrue(TEXT("Full save restores the authoritative ledger"),Mode->RestoreSnapshot(Loaded,false));
 TestTrue(TEXT("Repeated restore is idempotent"),Mode->RestoreSnapshot(Loaded,false));
 TestEqual(TEXT("Delivery stage restored from persisted receipts"),Player->DeliveryMission->GetStage(),EAskirDeliveryStage::ReturnToTorben);
 TestEqual(TEXT("Bargain choice restored"),Player->DeliveryMission->GetAcceptedChoice(),FName(TEXT("bargain")));
 TestEqual(TEXT("Reputation restored without applying choice twice"),Reputation->GetReputation(TEXT("Freie Händler")),2);
 TestEqual(TEXT("Player wallet restored without reward minting"),Player->TradeComponent->Gold,23);
 TestEqual(TEXT("Current map merchant wallet restored"),Merchant->MerchantGold,540);
 TestEqual(TEXT("Current map merchant stock restored"),Merchant->Inventory[0].Stock,8);
 TestFalse(TEXT("Restored encounter cannot be credited twice"),Player->DeliveryMission->CreditBandit(TEXT("SAVE_A")));
 TestFalse(TEXT("Restored cargo cannot be recovered twice"),Player->DeliveryMission->RecoverCargo(TEXT("SAVE_CARGO")));
 Loaded->SavedDialogueFlags.Add(FName(TEXT("SQ_001_CHOICE_help")));
 TestFalse(TEXT("Conflicting choices reject the complete snapshot"),Loaded->ValidateSnapshot(Reason));
 TestFalse(TEXT("Corrupt save cannot partially restore player state"),Mode->RestoreSnapshot(Loaded,false));
 TestEqual(TEXT("Rejected snapshot preserves wallet"),Player->TradeComponent->Gold,23);
 TestEqual(TEXT("Rejected snapshot preserves merchant wallet"),Merchant->MerchantGold,540);
 Loaded->SavedDialogueFlags.Remove(FName(TEXT("SQ_001_CHOICE_help")));
 Loaded->SavedConsumedQuestTriggers.Add(FName(TEXT("SQ_001_KILL_UNEXPECTED_THIRD")));
 TestFalse(TEXT("Third bandit receipt cannot inflate the canonical encounter"),Loaded->ValidateSnapshot(Reason));
 Loaded->SavedConsumedQuestTriggers.Remove(FName(TEXT("SQ_001_KILL_UNEXPECTED_THIRD")));
 Loaded->SavedConsumedQuestTriggers.Remove(FName(TEXT("SQ_001_CARGO_SOURCE_SAVE_CARGO")));
 TestFalse(TEXT("Recovered cargo requires its stable source identity"),Loaded->ValidateSnapshot(Reason));
 Loaded->SavedConsumedQuestTriggers.Add(FName(TEXT("SQ_001_CARGO_SOURCE_SAVE_CARGO")));
 Loaded->SavedObjectivesByQuest[TEXT("SQ_001")].Objectives[2].CurrentCount=1;
 Loaded->SavedActiveObjectives[2].CurrentCount=1;
 TestFalse(TEXT("Unpaid return objective cannot survive as completed progress"),Loaded->ValidateSnapshot(Reason));
 Loaded->SavedObjectivesByQuest[TEXT("SQ_001")].Objectives[2].CurrentCount=0;
 Loaded->SavedActiveObjectives[2].CurrentCount=0;
 Loaded->SavedObjectivesByQuest[TEXT("SQ_001")].Objectives[0].CurrentCount=1;
 Loaded->SavedActiveObjectives[0].CurrentCount=1;
 TestFalse(TEXT("Matching mirrors cannot override two kill receipts"),Loaded->ValidateSnapshot(Reason));
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirGenericQuestSnapshotConsistencyTest,"Askir.Persistence.RejectContradictoryQuestMirrors",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirGenericQuestSnapshotConsistencyTest::RunTest(const FString& Parameters)
{
 auto* Save=NewObject<UAskirSaveGame>();FString Reason;
 TestTrue(TEXT("Existing v5 save without new SQ_001 receipts remains valid"),Save->ValidateSnapshot(Reason));
 Save->SavedQuestStates.Add(TEXT("SIDE_QUEST"),EAskirQuestState::Active);
 TestFalse(TEXT("Active quest needs its objective list"),Save->ValidateSnapshot(Reason));
 FAskirQuestObjective O;O.ObjectiveID=TEXT("COLLECT");O.RequiredCount=3;O.CurrentCount=1;
 Save->SavedObjectivesByQuest.FindOrAdd(TEXT("SIDE_QUEST")).Objectives.Add(O);
 TestFalse(TEXT("Authoritative objective list needs legacy HUD mirror"),Save->ValidateSnapshot(Reason));
 Save->SavedLegacyActiveQuestID=TEXT("SIDE_QUEST");Save->SavedActiveObjectives={O};
 TestTrue(TEXT("Consistent active quest accepted"),Save->ValidateSnapshot(Reason));
 Save->SavedActiveObjectives[0].CurrentCount=2;
 TestFalse(TEXT("Conflicting HUD mirror rejected"),Save->ValidateSnapshot(Reason));
 Save->SavedActiveObjectives[0].CurrentCount=1;
 Save->SavedQuestStates[TEXT("SIDE_QUEST")]=EAskirQuestState::Completed;
 TestFalse(TEXT("Completed quest cannot retain active objective list"),Save->ValidateSnapshot(Reason));
 Save->SavedQuestStates[TEXT("SIDE_QUEST")]=EAskirQuestState::Active;
 Save->SavedConsumedQuestTriggers.Add(NAME_None);
 TestFalse(TEXT("Empty quest trigger receipt rejected"),Save->ValidateSnapshot(Reason));
 Save->SavedConsumedQuestTriggers.Remove(NAME_None);Save->SavedDialogueFlags={FName(TEXT("CHOICE")),FName(TEXT("CHOICE"))};
 TestFalse(TEXT("Duplicate dialogue flags rejected"),Save->ValidateSnapshot(Reason));
 Save->SavedDialogueFlags.Reset();Save->SaveFormatVersion=2;Save->SavedObjectivesByQuest.Reset();
 TestTrue(TEXT("Legacy pre-v3 objective migration remains accepted"),Save->ValidateSnapshot(Reason));
 return true;
}
#endif
