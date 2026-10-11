#include "Economy/AskirTradeBridgeComponent.h"
#include "Economy/AskirMerchantComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
UAskirTradeBridgeComponent::UAskirTradeBridgeComponent(){PrimaryComponentTick.bCanEverTick=false;}
UAskirItemData* UAskirTradeBridgeComponent::FindItem(FName ItemID) const
{
 for(const auto& Entry:Catalog)if(Entry.ItemID==ItemID)return Entry.Item;
 return nullptr;
}
bool UAskirTradeBridgeComponent::BuyItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32& PlayerGold,float PriceMultiplier)
{
 if(!IsValid(Merchant)||!IsValid(PlayerInventory)||Quantity<=0||PlayerGold<0||!FMath::IsFinite(PriceMultiplier))return false;
 UAskirItemData* Item=FindItem(ItemID);
 if(!IsValid(Item))return false;
 const int32 BuyUnit=Merchant->GetUnitBuyPrice(ItemID,PriceMultiplier);
 const int64 BuyTotal=int64(BuyUnit)*Quantity;
 if(BuyUnit<=0||BuyTotal>MAX_int32||int64(Merchant->MerchantGold)+BuyTotal>MAX_int32||BuyTotal>PlayerGold)return false;
 // Commit the merchant trade only after validating the catalog and inventory target.
 if(!Merchant->BuyFromMerchant(ItemID,Quantity,PlayerGold,PriceMultiplier))return false;
 if(PlayerInventory->AddItem(Item,Quantity))return true;
 // Compensating rollback; avoid relying on the merchant sell-back price.
 const int32 Unit=Merchant->GetUnitBuyPrice(ItemID,PriceMultiplier);
 const int64 Refund=int64(Unit)*Quantity;
 for(auto& Entry:Merchant->Inventory)if(Entry.ItemID==ItemID){Entry.Stock+=Quantity;break;}
 Merchant->MerchantGold-=int32(Refund);PlayerGold+=int32(Refund);
 return false;
}
bool UAskirTradeBridgeComponent::SellItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32& PlayerGold,float PriceMultiplier)
{
 if(!IsValid(Merchant)||!IsValid(PlayerInventory)||Quantity<=0||PlayerGold<0||!FMath::IsFinite(PriceMultiplier))return false;
 UAskirItemData* Item=FindItem(ItemID);
 if(!IsValid(Item)||!PlayerInventory->HasItem(Item,Quantity))return false;
 const int32 Unit=Merchant->GetUnitSellPrice(ItemID,PriceMultiplier);
 const int64 Total=int64(Unit)*Quantity;
 if(Unit<=0||Total>Merchant->MerchantGold||Total>MAX_int32||int64(PlayerGold)+Total>MAX_int32)return false;
 if(!PlayerInventory->RemoveItem(Item,Quantity))return false;
 if(Merchant->SellToMerchant(ItemID,Quantity,PlayerGold,PriceMultiplier))return true;
 // Merchant rejected transaction: restore the player's item stack.
 PlayerInventory->AddItem(Item,Quantity);
 return false;
}
