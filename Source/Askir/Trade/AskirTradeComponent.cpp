#include "Trade/AskirTradeComponent.h"
#include "Core/AskirCheckedMath.h"

void UAskirTradeComponent::EarnGold(int32 Amount){Gold=AskirCheckedMath::SaturatingNonNegativeAdd(Gold,Amount);}
void UAskirTradeComponent::SpendGold(int32 Amount){if(Amount>0)Gold=FMath::Max(0,FMath::Max(0,Gold)-Amount);}
void UAskirTradeComponent::RaiseFavor(int32 Amount){MerchantFavor=AskirCheckedMath::ClampAdd(MerchantFavor,Amount,-100,100);}
void UAskirTradeComponent::RecordTrade(int32 Value){TradeVolume=AskirCheckedMath::SaturatingNonNegativeAdd(TradeVolume,Value);}
void UAskirTradeComponent::PayFine(int32 Amount){if(Amount<=0)return;SpendGold(Amount);MerchantFavor=AskirCheckedMath::ClampAdd(MerchantFavor,1,-100,100);}
