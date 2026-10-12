#include "Economy/AskirEconomyComponent.h"
#include "Core/AskirCheckedMath.h"
UAskirEconomyComponent::UAskirEconomyComponent(){PrimaryComponentTick.bCanEverTick=false;}
void UAskirEconomyComponent::SimulateMarketDay()
{
    for(auto& Entry:Market)
    {
        const double Scarcity=Entry.Stock<=0?2.0:FMath::Clamp(1.0+(10.0-double(Entry.Stock))*.025,.5,2.0);
        Entry.CurrentPrice=FMath::IsFinite(Entry.BasePrice)&&Entry.BasePrice>0.f?float(FMath::Clamp(double(Entry.BasePrice)*Scarcity,.01,double(MAX_int32))):0.f;
    }
}
float UAskirEconomyComponent::GetPrice(FName ID) const
{if(ID.IsNone())return 0.f;for(const auto& Entry:Market)if(Entry.ItemID==ID)return FMath::IsFinite(Entry.CurrentPrice)&&Entry.CurrentPrice>0.f?Entry.CurrentPrice:0.f;return 0.f;}
bool UAskirEconomyComponent::Buy(FName ID,int32 Quantity,float& Cost)
{
    Cost=0.f;
    int32 ChargedGold=0;
    if(Gold<0||!AskirCheckedMath::TryCeilGold(GetPrice(ID),Quantity,ChargedGold)||ChargedGold>Gold)return false;
    FAskirMarketEntry* Stock=nullptr;
    for(auto& Entry:Market)if(Entry.ItemID==ID){if(Stock)return false;Stock=&Entry;}
    if(!Stock||Stock->Stock<Quantity)return false;
    Stock->Stock-=Quantity;Gold-=ChargedGold;Cost=float(ChargedGold);
    return true;
}
