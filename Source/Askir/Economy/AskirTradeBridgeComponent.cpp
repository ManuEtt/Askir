#include "Economy/AskirTradeBridgeComponent.h"
#include "Economy/AskirMerchantComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Core/AskirCheckedMath.h"

UAskirTradeBridgeComponent::UAskirTradeBridgeComponent(){PrimaryComponentTick.bCanEverTick=false;}
UAskirItemData* UAskirTradeBridgeComponent::FindItem(FName ItemID) const
{
    if(ItemID.IsNone())return nullptr;
    UAskirItemData* Item=nullptr;
    for(const auto& Entry:Catalog)if(Entry.ItemID==ItemID)
    {
        // Ambiguous IDs must never select one item while pricing another.
        if(Item||!IsValid(Entry.Item)||Entry.Item->ItemID!=ItemID)return nullptr;
        Item=Entry.Item;
    }
    return Item;
}
bool UAskirTradeBridgeComponent::BuyItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32& PlayerGold,float PriceMultiplier)
{
    if(!IsValid(Merchant)||!IsValid(PlayerInventory)||Merchant->bTransactionInProgress||Quantity<=0||PlayerGold<0||Merchant->MerchantGold<0||!FMath::IsFinite(PriceMultiplier))return false;
    UAskirItemData* Item=FindItem(ItemID);
    if(!IsValid(Item))return false;
    FAskirShopItem* Stock=nullptr;
    for(auto& Entry:Merchant->Inventory)if(Entry.ItemID==ItemID){if(Stock)return false;Stock=&Entry;}
    int32 Total=0;
    if(!Stock||Stock->Stock<Quantity||!AskirCheckedMath::TryPositiveProduct(Merchant->GetUnitBuyPrice(ItemID,PriceMultiplier),Quantity,Total)||Total>PlayerGold||!AskirCheckedMath::CanCredit(Merchant->MerchantGold,Total))return false;
    FAskirInventoryEntry Added;Added.Item=Item;Added.Quantity=Quantity;
    TArray<FAskirInventoryEntry> Prepared;
    if(!PlayerInventory->PrepareExchange({}, {Added},Prepared))return false;
    // No delegate runs until all three ledgers have committed; callbacks cannot reenter.
    TGuardValue<bool> MerchantGuard(Merchant->bTransactionInProgress,true);
    TGuardValue<bool> InventoryGuard(PlayerInventory->bTransactionInProgress,true);
    PlayerInventory->Items=MoveTemp(Prepared);
    Stock->Stock-=Quantity;PlayerGold-=Total;Merchant->MerchantGold+=Total;
    PlayerInventory->OnInventoryChanged.Broadcast();
    Merchant->OnTradeCompleted.Broadcast(ItemID,Quantity,Total);
    return true;
}
bool UAskirTradeBridgeComponent::SellItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32& PlayerGold,float PriceMultiplier)
{
    if(!IsValid(Merchant)||!IsValid(PlayerInventory)||Merchant->bTransactionInProgress||Quantity<=0||PlayerGold<0||Merchant->MerchantGold<0||!FMath::IsFinite(PriceMultiplier))return false;
    UAskirItemData* Item=FindItem(ItemID);
    if(!IsValid(Item))return false;
    FAskirShopItem* Stock=nullptr;
    for(auto& Entry:Merchant->Inventory)if(Entry.ItemID==ItemID){if(Stock)return false;Stock=&Entry;}
    int32 Total=0;
    if(!Stock||!AskirCheckedMath::TryPositiveProduct(Merchant->GetUnitSellPrice(ItemID,PriceMultiplier),Quantity,Total)||Total>Merchant->MerchantGold||!AskirCheckedMath::CanCredit(PlayerGold,Total)||!AskirCheckedMath::CanCredit(Stock->Stock,Quantity))return false;
    FAskirInventoryEntry Removed;Removed.Item=Item;Removed.Quantity=Quantity;
    TArray<FAskirInventoryEntry> Prepared;
    if(!PlayerInventory->PrepareExchange({Removed}, {},Prepared))return false;
    TGuardValue<bool> MerchantGuard(Merchant->bTransactionInProgress,true);
    TGuardValue<bool> InventoryGuard(PlayerInventory->bTransactionInProgress,true);
    PlayerInventory->Items=MoveTemp(Prepared);
    Stock->Stock+=Quantity;PlayerGold+=Total;Merchant->MerchantGold-=Total;
    PlayerInventory->OnInventoryChanged.Broadcast();
    Merchant->OnTradeCompleted.Broadcast(ItemID,-Quantity,Total);
    return true;
}
