#include "Economy/AskirTradeBridgeComponent.h"
#include "Economy/AskirMerchantComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Economy/AskirEconomyRules.h"
#include "Equipment/AskirEquipmentComponent.h"
#include "GameFramework/Actor.h"

namespace
{
    bool OwnsInventoryLedgerReference(const UAskirInventoryComponent* Inventory,const int32& Value)
    {
        for(const auto& Entry:Inventory->Items)if(&Value==&Entry.Quantity)return true;
        return false;
    }
}

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
bool UAskirTradeBridgeComponent::PrepareTrade(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32 PlayerGold,float PriceMultiplier,AskirEconomyRules::Direction Operation,AskirEconomyRules::TradePlan& Plan,int32& StockIndex,TArray<FAskirInventoryEntry>& PreparedInventory) const
{
    if(bTradeInProgress||!IsValid(Merchant)||!IsValid(PlayerInventory)||!Merchant->CanAcceptTransactions())return false;
    UAskirItemData* Item=FindItem(ItemID);
    if(!IsValid(Item)||Item->MaxStackSize<=0)return false;
    StockIndex=INDEX_NONE;
    for(int32 Index=0;Index<Merchant->Inventory.Num();++Index)if(Merchant->Inventory[Index].ItemID==ItemID)
    {
        if(StockIndex!=INDEX_NONE)return false;
        StockIndex=Index;
    }
    if(StockIndex==INDEX_NONE)return false;
    const auto& Stock=Merchant->Inventory[StockIndex];
    const int32 Price=Operation==AskirEconomyRules::Direction::Buy?
        Merchant->GetUnitBuyPrice(ItemID,PriceMultiplier):Merchant->GetUnitSellPrice(ItemID,PriceMultiplier);
    if(!AskirEconomyRules::PrepareTrade(Operation,Price,Quantity,PlayerGold,Merchant->MerchantGold,Stock.Stock,Plan))return false;
    FAskirInventoryEntry Entry;Entry.Item=Item;Entry.Quantity=Quantity;
    return Operation==AskirEconomyRules::Direction::Buy?
        PlayerInventory->PrepareExchange({}, {Entry},PreparedInventory):
        PlayerInventory->PrepareExchange({Entry}, {},PreparedInventory);
}
bool UAskirTradeBridgeComponent::CanBuyItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32 PlayerGold,float PriceMultiplier) const
{
    AskirEconomyRules::TradePlan Plan;int32 StockIndex=INDEX_NONE;TArray<FAskirInventoryEntry> Prepared;
    return PrepareTrade(Merchant,PlayerInventory,ItemID,Quantity,PlayerGold,PriceMultiplier,AskirEconomyRules::Direction::Buy,Plan,StockIndex,Prepared);
}
bool UAskirTradeBridgeComponent::CanSellItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32 PlayerGold,float PriceMultiplier) const
{
    AskirEconomyRules::TradePlan Plan;int32 StockIndex=INDEX_NONE;TArray<FAskirInventoryEntry> Prepared;
    return PrepareTrade(Merchant,PlayerInventory,ItemID,Quantity,PlayerGold,PriceMultiplier,AskirEconomyRules::Direction::Sell,Plan,StockIndex,Prepared);
}
bool UAskirTradeBridgeComponent::BuyItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32& PlayerGold,float PriceMultiplier)
{
    if(bTradeInProgress||!IsValid(Merchant)||!Merchant->CanAcceptTransactions()||!Merchant->CaptureInitialState())return false;
    AskirEconomyRules::TradePlan Plan;int32 StockIndex=INDEX_NONE;TArray<FAskirInventoryEntry> Prepared;
    if(!PrepareTrade(Merchant,PlayerInventory,ItemID,Quantity,PlayerGold,PriceMultiplier,AskirEconomyRules::Direction::Buy,Plan,StockIndex,Prepared))return false;
    // A caller must supply its own wallet, never a reference to a merchant ledger.
    if(Merchant->OwnsLedgerReference(PlayerGold)||OwnsInventoryLedgerReference(PlayerInventory,PlayerGold))return false;
    // No delegate runs until all three ledgers have committed; callbacks cannot reenter.
    TGuardValue<bool> BridgeGuard(bTradeInProgress,true);
    TGuardValue<bool> MerchantGuard(Merchant->bTransactionInProgress,true);
    TGuardValue<bool> InventoryGuard(PlayerInventory->bTransactionInProgress,true);
    PlayerInventory->Items=MoveTemp(Prepared);
    Merchant->Inventory[StockIndex].Stock=Plan.Stock;PlayerGold=Plan.PlayerGold;Merchant->MerchantGold=Plan.MerchantGold;
    if(AActor* Owner=PlayerInventory->GetOwner())
        if(auto* Equipment=Owner->FindComponentByClass<UAskirEquipmentComponent>())Equipment->ReconcileWithInventory();
    PlayerInventory->OnInventoryChanged.Broadcast();
    Merchant->OnTradeCompleted.Broadcast(ItemID,Quantity,Plan.TotalGold);
    return true;
}
bool UAskirTradeBridgeComponent::SellItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32& PlayerGold,float PriceMultiplier)
{
    if(bTradeInProgress||!IsValid(Merchant)||!Merchant->CanAcceptTransactions()||!Merchant->CaptureInitialState())return false;
    AskirEconomyRules::TradePlan Plan;int32 StockIndex=INDEX_NONE;TArray<FAskirInventoryEntry> Prepared;
    if(!PrepareTrade(Merchant,PlayerInventory,ItemID,Quantity,PlayerGold,PriceMultiplier,AskirEconomyRules::Direction::Sell,Plan,StockIndex,Prepared))return false;
    if(Merchant->OwnsLedgerReference(PlayerGold)||OwnsInventoryLedgerReference(PlayerInventory,PlayerGold))return false;
    TGuardValue<bool> BridgeGuard(bTradeInProgress,true);
    TGuardValue<bool> MerchantGuard(Merchant->bTransactionInProgress,true);
    TGuardValue<bool> InventoryGuard(PlayerInventory->bTransactionInProgress,true);
    PlayerInventory->Items=MoveTemp(Prepared);
    Merchant->Inventory[StockIndex].Stock=Plan.Stock;PlayerGold=Plan.PlayerGold;Merchant->MerchantGold=Plan.MerchantGold;
    if(AActor* Owner=PlayerInventory->GetOwner())
        if(auto* Equipment=Owner->FindComponentByClass<UAskirEquipmentComponent>())Equipment->ReconcileWithInventory();
    PlayerInventory->OnInventoryChanged.Broadcast();
    Merchant->OnTradeCompleted.Broadcast(ItemID,-Quantity,Plan.TotalGold);
    return true;
}
