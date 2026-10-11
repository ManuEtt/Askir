#include "AskirTradeComponent.h"

void UAskirTradeComponent::EarnGold(int32 Amount) { Gold=FMath::Max(0,Gold+FMath::Max(0,Amount)); }
void UAskirTradeComponent::SpendGold(int32 Amount) { Gold=FMath::Max(0,Gold-FMath::Max(0,Amount)); }
void UAskirTradeComponent::RaiseFavor(int32 Amount) { MerchantFavor=FMath::Clamp(MerchantFavor+Amount,-100,100); }
void UAskirTradeComponent::RecordTrade(int32 Value) { TradeVolume+=FMath::Max(0,Value); }
void UAskirTradeComponent::PayFine(int32 Amount) { Gold=FMath::Max(0,Gold-FMath::Max(0,Amount));MerchantFavor=FMath::Min(100,MerchantFavor+1); }
