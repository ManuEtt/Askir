#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Characters/AskirCharacter.h"
#include "Economy/AskirMerchantComponent.h"
#include "Economy/AskirTradeBridgeComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Equipment/AskirEquipmentComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirSoldEquipmentTest,"Askir.Economy.SoldEquipmentReconciles",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirSoldEquipmentTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    auto* Player=Fixture.Spawn<AAskirCharacter>();
    if(!TestNotNull(TEXT("Spawned player"),Player))return false;
    auto* Sword=NewObject<UAskirItemData>();Sword->ItemID=TEXT("TestSword");
    Sword->ItemType=EAskirItemType::Weapon;Sword->MaxStackSize=1;Sword->Damage=24.f;
    FAskirTradeCatalogEntry Catalog;Catalog.ItemID=Sword->ItemID;Catalog.Item=Sword;
    Player->TradeBridgeComponent->Catalog={Catalog};
    TestTrue(TEXT("Seed two owned swords"),Player->InventoryComponent->AddItem(Sword,2));
    TestTrue(TEXT("Equip owned sword"),Player->EquipmentComponent->TryEquipInventoryItem(Sword));
    auto* Merchant=NewObject<UAskirMerchantComponent>();Merchant->MerchantGold=100;
    FAskirShopItem Offer;Offer.ItemID=Sword->ItemID;Offer.BasePrice=20;Offer.Stock=0;
    Merchant->Inventory={Offer};int32 Gold=0;
    TestTrue(TEXT("Sell first copy"),Player->TradeBridgeComponent->SellItem(Merchant,Player->InventoryComponent,Sword->ItemID,1,Gold));
    TestEqual(TEXT("Remaining owned copy retains equipment"),Player->EquipmentComponent->EquippedWeaponID,Sword->ItemID);
    TestTrue(TEXT("Sell final copy"),Player->TradeBridgeComponent->SellItem(Merchant,Player->InventoryComponent,Sword->ItemID,1,Gold));
    TestTrue(TEXT("No orphaned equipped ID"),Player->EquipmentComponent->EquippedWeaponID.IsNone());
    TestEqual(TEXT("No damage bonus after final sale"),Player->EquipmentComponent->WeaponRating,0.f);
    TestEqual(TEXT("Gold credited only for owned copies"),Gold,20);
    TestFalse(TEXT("Repeated sale cannot pay twice"),Player->TradeBridgeComponent->SellItem(Merchant,Player->InventoryComponent,Sword->ItemID,1,Gold));
    return true;
}
#endif
