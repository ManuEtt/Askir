#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirTradeComponent.generated.h"

UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirTradeComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Trade") int32 Gold=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Trade") int32 MerchantFavor=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Trade") int32 TradeVolume=0;
 UFUNCTION(BlueprintCallable,Category="Askir|Trade") void EarnGold(int32 Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Trade") void SpendGold(int32 Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Trade") void RaiseFavor(int32 Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Trade") void RecordTrade(int32 Value);
 UFUNCTION(BlueprintCallable,Category="Askir|Trade") void PayFine(int32 Amount);
};
