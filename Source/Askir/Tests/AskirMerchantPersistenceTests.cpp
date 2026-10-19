#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Tests/AskirTransactionPersistenceProbe.h"
#include "Economy/AskirMerchantComponent.h"
#include "Economy/AskirTradeBridgeComponent.h"
#include "Characters/AskirCharacter.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Trade/AskirTradeComponent.h"
#include "Game/AskirGameMode.h"
#include "Save/AskirSaveGame.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Package.h"

namespace
{
FAskirShopItem ShopRow(FName ID,int32 Stock=10)
{
    FAskirShopItem Row;Row.ItemID=ID;Row.BasePrice=20;Row.Stock=Stock;Row.MaxStock=10;return Row;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirMerchantSnapshotValidationTest,"Askir.Persistence.MerchantSnapshotIsAtomic",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirMerchantSnapshotValidationTest::RunTest(const FString& Parameters)
{
    auto* Merchant=NewObject<UAskirMerchantComponent>();
    Merchant->Inventory.Add(ShopRow(TEXT("Ore")));
    Merchant->MerchantGold=500;
    TestTrue(TEXT("Initial state captured"),Merchant->CaptureInitialState());
    FAskirMerchantSnapshot Saved;
    TestTrue(TEXT("Capture actual wallet and stock"),Merchant->CaptureSnapshot(Saved));
    Merchant->MerchantGold=700;Merchant->Inventory[0].Stock=3;
    FAskirMerchantSnapshot Invalid=Saved;Invalid.StockByItem[TEXT("Ore")]=-1;
    TestFalse(TEXT("Negative stock rejects complete restore"),Merchant->RestoreSnapshot(Invalid));
    TestEqual(TEXT("Rejected restore leaves wallet unchanged"),Merchant->MerchantGold,700);
    TestEqual(TEXT("Rejected restore leaves stock unchanged"),Merchant->Inventory[0].Stock,3);
    Invalid=Saved;Invalid.StockByItem.Remove(TEXT("Ore"));Invalid.StockByItem.Add(TEXT("Unknown"),10);
    TestFalse(TEXT("Different catalog IDs cannot restore"),Merchant->RestoreSnapshot(Invalid));
    TestTrue(TEXT("Valid snapshot restores"),Merchant->RestoreSnapshot(Saved));
    TestTrue(TEXT("Repeated restore is idempotent"),Merchant->RestoreSnapshot(Saved));
    TestEqual(TEXT("Wallet restored exactly once"),Merchant->MerchantGold,500);
    TestEqual(TEXT("Stock restored exactly"),Merchant->Inventory[0].Stock,10);
    Merchant->Inventory[0].Stock=15;Merchant->Restock(1);
    TestEqual(TEXT("Restock preserves sold-back stock above target"),Merchant->Inventory[0].Stock,15);
    TestFalse(TEXT("Merchant wallet cannot alias player wallet"),Merchant->BuyFromMerchant(TEXT("Ore"),1,Merchant->MerchantGold));
    TestFalse(TEXT("Catalog price cannot alias player wallet"),Merchant->SellToMerchant(TEXT("Ore"),1,Merchant->Inventory[0].BasePrice));
    Merchant->Inventory.Add(ShopRow(TEXT("Ore")));
    TestEqual(TEXT("Ambiguous item ID has no price"),Merchant->GetUnitBuyPrice(TEXT("Ore")),0);
    TestFalse(TEXT("Duplicate ID cannot capture"),Merchant->CaptureSnapshot(Saved));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirMerchantSaveRoundTripTest,"Askir.Persistence.MerchantMapLedgerRoundTrip",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirMerchantSaveRoundTripTest::RunTest(const FString& Parameters)
{
    auto* Save=NewObject<UAskirSaveGame>();
    FAskirMerchantSnapshot Ardem;Ardem.Gold=521;Ardem.StockByItem.Add(TEXT("Ore"),7);
    FAskirMerchantSnapshot Thalor;Thalor.Gold=310;Thalor.StockByItem.Add(TEXT("Ore"),2);
    Save->SavedMerchantsByMap.FindOrAdd(TEXT("Dornfeld_V79")).Merchants.Add(TEXT("GENERAL_STORE"),Ardem);
    Save->SavedMerchantsByMap.FindOrAdd(TEXT("ThalorHarbour")).Merchants.Add(TEXT("GENERAL_STORE"),Thalor);
    FString Reason;TestTrue(TEXT("Same merchant ID on different maps is valid"),Save->ValidateSnapshot(Reason));
    TArray<uint8> Bytes;
    if(!TestTrue(TEXT("Actual SaveGame serializes nested merchant ledger"),UGameplayStatics::SaveGameToMemory(Save,Bytes)))return false;
    auto* Restored=Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
    if(!TestNotNull(TEXT("Actual SaveGame deserializes"),Restored))return false;
    TestEqual(TEXT("Ardem wallet preserved"),Restored->SavedMerchantsByMap[TEXT("Dornfeld_V79")].Merchants[TEXT("GENERAL_STORE")].Gold,521);
    TestEqual(TEXT("Thalor stock is isolated"),Restored->SavedMerchantsByMap[TEXT("ThalorHarbour")].Merchants[TEXT("GENERAL_STORE")].StockByItem[TEXT("Ore")],2);
    Restored->SavedMerchantsByMap[TEXT("Dornfeld_V79")].Merchants[TEXT("GENERAL_STORE")].Gold=-1;
    TestFalse(TEXT("One invalid merchant rejects complete save"),Restored->ValidateSnapshot(Reason));
    Restored->SaveFormatVersion=4;
    TestTrue(TEXT("V4 ignores absent-v5 fields for explicit migration"),Restored->ValidateSnapshot(Reason));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirMerchantPlayerSaveIntegrationTest,"Askir.Persistence.PlayerMerchantTradeAndRepeatedLoad",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirMerchantPlayerSaveIntegrationTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Initialized runtime test world"),Fixture.World))return false;
    auto* Player=Fixture.Spawn<AAskirCharacter>(FVector(0.f,0.f,96.f));
    auto* Controller=Fixture.Spawn<APlayerController>();Controller->Possess(Player);
    auto* Mode=Fixture.Spawn<AAskirGameMode>();
    auto* Store=Fixture.Spawn<AActor>();
    // Configure before RegisterComponent/BeginPlay freezes the authored defaults.
    auto* Merchant=NewObject<UAskirMerchantComponent>(Store);
    Merchant->MerchantID=TEXT("DORNFELD_GENERAL_STORE");Merchant->Inventory.Add(ShopRow(TEXT("Ore")));Merchant->MerchantGold=500;
    Store->AddInstanceComponent(Merchant);Merchant->RegisterComponent();
    auto* Bridge=Fixture.AddComponent<UAskirTradeBridgeComponent>(Player);
    UPackage* Package=CreatePackage(*(TEXT("/Game/Askir/Automation/")+FGuid::NewGuid().ToString(EGuidFormats::Digits)));
    auto* Ore=NewObject<UAskirItemData>(Package,TEXT("Ore"),RF_Public);Ore->ItemID=TEXT("Ore");Ore->MaxStackSize=99;
    FAskirTradeCatalogEntry Catalog;Catalog.ItemID=Ore->ItemID;Catalog.Item=Ore;Bridge->Catalog.Add(Catalog);
    Player->TradeComponent->Gold=100;
    if(!TestTrue(TEXT("Buy two actual inventory items"),Bridge->BuyItem(Merchant,Player->InventoryComponent,Ore->ItemID,2,Player->TradeComponent->Gold)))return false;
    auto* Save=Mode->CaptureSnapshot();
    if(!TestNotNull(TEXT("Full player and merchant snapshot captured"),Save))return false;
    auto* Probe=NewObject<UAskirTransactionPersistenceProbe>();Probe->Mode=Mode;Probe->Snapshot=Save;
    Player->InventoryComponent->OnInventoryChanged.AddDynamic(Probe,&UAskirTransactionPersistenceProbe::ObserveInventoryChange);
    TestTrue(TEXT("Change all ledgers after snapshot"),Bridge->BuyItem(Merchant,Player->InventoryComponent,Ore->ItemID,1,Player->TradeComponent->Gold));
    Player->InventoryComponent->OnInventoryChanged.RemoveDynamic(Probe,&UAskirTransactionPersistenceProbe::ObserveInventoryChange);
    TestTrue(TEXT("Actual inventory transaction delegate observed"),Probe->bCallbackObserved);
    TestTrue(TEXT("Save capture during delegate is blocked"),Probe->bCaptureBlocked);
    TestTrue(TEXT("Restore during delegate is blocked"),Probe->bRestoreBlocked);
    TestEqual(TEXT("Delegate cannot rewind completed trade"),Player->TradeComponent->Gold,40);
    TestNotNull(TEXT("Capture succeeds after the transaction has closed"),Mode->CaptureSnapshot());
    TestTrue(TEXT("Restore saved transaction ledgers"),Mode->RestoreSnapshot(Save,false));
    TestTrue(TEXT("Repeat load without minting gold or items"),Mode->RestoreSnapshot(Save,false));
    TestEqual(TEXT("Player wallet restored"),Player->TradeComponent->Gold,60);
    TestEqual(TEXT("Merchant wallet restored"),Merchant->MerchantGold,540);
    TestEqual(TEXT("Merchant stock restored"),Merchant->Inventory[0].Stock,8);
    TestEqual(TEXT("Inventory restored"),Player->InventoryComponent->GetItemCount(Ore),2);
    const FName Map(*UGameplayStatics::GetCurrentLevelName(Fixture.World,true));
    FAskirMerchantSnapshot Good=Save->SavedMerchantsByMap[Map].Merchants[Merchant->MerchantID];
    Save->SavedMerchantsByMap[Map].Merchants[Merchant->MerchantID].StockByItem[Ore->ItemID]=-1;
    TestFalse(TEXT("Invalid ledger rejected before player changes"),Mode->RestoreSnapshot(Save,false));
    TestEqual(TEXT("Rejected load retains player wallet"),Player->TradeComponent->Gold,60);
    TestEqual(TEXT("Rejected load retains merchant wallet"),Merchant->MerchantGold,540);
    Save->SavedMerchantsByMap[Map].Merchants.Remove(Merchant->MerchantID);
    TestFalse(TEXT("V5 missing same-map receipt cannot refill stock"),Mode->RestoreSnapshot(Save,false));
    Save->SavedMerchantsByMap[Map].Merchants.Add(Merchant->MerchantID,Good);
    Merchant->MerchantID=NAME_None;
    TestNull(TEXT("Missing stable merchant identity blocks capture"),Mode->CaptureSnapshot());
    Merchant->MerchantID=TEXT("DORNFELD_GENERAL_STORE");
    Save->SaveFormatVersion=4;
    TestTrue(TEXT("Legacy save migration succeeds"),Mode->RestoreSnapshot(Save,false));
    TestEqual(TEXT("Legacy load clears live merchant wallet to authored initial"),Merchant->MerchantGold,500);
    TestEqual(TEXT("Legacy load clears live stock to authored initial"),Merchant->Inventory[0].Stock,10);
    return true;
}
#endif
