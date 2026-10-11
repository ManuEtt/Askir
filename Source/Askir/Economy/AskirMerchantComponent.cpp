#include "Economy/AskirMerchantComponent.h"
UAskirMerchantComponent::UAskirMerchantComponent(){PrimaryComponentTick.bCanEverTick=false;}
int32 UAskirMerchantComponent::GetUnitBuyPrice(FName ID,float Multiplier) const
{
 for(const auto& I:Inventory)if(I.ItemID==ID)
 return FMath::Max(1,FMath::RoundToInt(FMath::Clamp(double(I.BasePrice)*double(FMath::Clamp(FMath::IsFinite(Multiplier)?Multiplier:1.f,.5f,2.f)),1.0,double(MAX_int32))));
 return 0;
}
int32 UAskirMerchantComponent::GetUnitSellPrice(FName ID,float Multiplier) const
{
 const int32 Buy=GetUnitBuyPrice(ID,Multiplier);
 return Buy>0?FMath::Max(1,FMath::FloorToInt(Buy*FMath::Clamp(FMath::IsFinite(SellBackRatio)?SellBackRatio:.5f,.01f,1.f))):0;
}
bool UAskirMerchantComponent::BuyFromMerchant(FName ID,int32 Quantity,int32& PlayerGold,float Multiplier)
{
 if(Quantity<=0)return false;
 for(auto& I:Inventory)if(I.ItemID==ID)
 {
  const int32 Unit=GetUnitBuyPrice(ID,Multiplier);
  const int64 Total=int64(Unit)*Quantity;
  if(Unit<=0||I.Stock<Quantity||Total>PlayerGold||Total>MAX_int32||int64(MerchantGold)+Total>MAX_int32)return false;
  I.Stock-=Quantity;PlayerGold-=int32(Total);MerchantGold+=int32(Total);
  OnTradeCompleted.Broadcast(ID,Quantity,int32(Total));return true;
 }
 return false;
}
bool UAskirMerchantComponent::SellToMerchant(FName ID,int32 Quantity,int32& PlayerGold,float Multiplier)
{
 if(Quantity<=0)return false;
 for(auto& I:Inventory)if(I.ItemID==ID)
 {
  const int32 Unit=GetUnitSellPrice(ID,Multiplier);
  const int64 Total=int64(Unit)*Quantity;
  if(Unit<=0||Total>MerchantGold||Total>MAX_int32||int64(PlayerGold)+Total>MAX_int32||I.Stock>MAX_int32-Quantity)return false;
  I.Stock+=Quantity;MerchantGold-=int32(Total);PlayerGold+=int32(Total);
  OnTradeCompleted.Broadcast(ID,-Quantity,int32(Total));return true;
 }
 return false;
}
void UAskirMerchantComponent::Restock(int32 UnitsPerItem)
{
 if(UnitsPerItem<=0)return;
 for(auto& I:Inventory)I.Stock=int32(FMath::Min(int64(FMath::Max(0,I.MaxStock)),int64(I.Stock)+int64(UnitsPerItem)));
}
