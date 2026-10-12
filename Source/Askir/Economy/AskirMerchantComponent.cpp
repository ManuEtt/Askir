#include "Economy/AskirMerchantComponent.h"
#include "Core/AskirCheckedMath.h"

UAskirMerchantComponent::UAskirMerchantComponent(){PrimaryComponentTick.bCanEverTick=false;}
int32 UAskirMerchantComponent::GetUnitBuyPrice(FName ID,float Multiplier) const
{
    if(ID.IsNone()||!FMath::IsFinite(Multiplier))return 0;
    for(const auto& Item:Inventory)if(Item.ItemID==ID)
    {
        if(Item.BasePrice<=0)return 0;
        const double Price=double(Item.BasePrice)*FMath::Clamp(double(Multiplier),.5,2.0);
        return int32(FMath::Clamp(std::floor(Price+.5),1.0,double(MAX_int32)));
    }
    return 0;
}
int32 UAskirMerchantComponent::GetUnitSellPrice(FName ID,float Multiplier) const
{
    const int32 Buy=GetUnitBuyPrice(ID,Multiplier);
    if(Buy<=0||!FMath::IsFinite(SellBackRatio))return 0;
    return int32(FMath::Clamp(std::floor(double(Buy)*FMath::Clamp(double(SellBackRatio),.01,1.0)),1.0,double(MAX_int32)));
}
bool UAskirMerchantComponent::BuyFromMerchant(FName ID,int32 Quantity,int32& PlayerGold,float Multiplier)
{
    if(bTransactionInProgress||Quantity<=0||PlayerGold<0||MerchantGold<0)return false;
    for(auto& Item:Inventory)if(Item.ItemID==ID)
    {
        int32 Total=0;
        if(!AskirCheckedMath::TryPositiveProduct(GetUnitBuyPrice(ID,Multiplier),Quantity,Total)||Item.Stock<Quantity||Total>PlayerGold||!AskirCheckedMath::CanCredit(MerchantGold,Total))return false;
        TGuardValue<bool> Guard(bTransactionInProgress,true);
        Item.Stock-=Quantity;PlayerGold-=Total;MerchantGold+=Total;
        OnTradeCompleted.Broadcast(ID,Quantity,Total);
        return true;
    }
    return false;
}
bool UAskirMerchantComponent::SellToMerchant(FName ID,int32 Quantity,int32& PlayerGold,float Multiplier)
{
    if(bTransactionInProgress||Quantity<=0||PlayerGold<0||MerchantGold<0)return false;
    for(auto& Item:Inventory)if(Item.ItemID==ID)
    {
        int32 Total=0;
        if(Item.Stock<0||!AskirCheckedMath::TryPositiveProduct(GetUnitSellPrice(ID,Multiplier),Quantity,Total)||Total>MerchantGold||!AskirCheckedMath::CanCredit(PlayerGold,Total)||!AskirCheckedMath::CanCredit(Item.Stock,Quantity))return false;
        TGuardValue<bool> Guard(bTransactionInProgress,true);
        Item.Stock+=Quantity;MerchantGold-=Total;PlayerGold+=Total;
        OnTradeCompleted.Broadcast(ID,-Quantity,Total);
        return true;
    }
    return false;
}
void UAskirMerchantComponent::Restock(int32 UnitsPerItem)
{
    if(bTransactionInProgress||UnitsPerItem<=0)return;
    for(auto& Item:Inventory)Item.Stock=int32(FMath::Min(int64(FMath::Max(0,Item.MaxStock)),int64(FMath::Max(0,Item.Stock))+UnitsPerItem));
}
