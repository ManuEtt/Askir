#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirMerchantComponent.generated.h"
USTRUCT(BlueprintType)
struct FAskirShopItem
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName ItemID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 BasePrice=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Stock=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 MaxStock=10;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAskirShopTrade,FName,ItemID,int32,Quantity,int32,TotalGold);
UCLASS(ClassGroup=(Askir),meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirMerchantComponent:public UActorComponent
{
 GENERATED_BODY()
public:
 UAskirMerchantComponent();
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Merchant") TArray<FAskirShopItem> Inventory;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Merchant") int32 MerchantGold=500;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Merchant") float SellBackRatio=.5f;
 UPROPERTY(BlueprintAssignable,Category="Askir|Merchant") FAskirShopTrade OnTradeCompleted;
 UFUNCTION(BlueprintPure,Category="Askir|Merchant") int32 GetUnitBuyPrice(FName ItemID,float ReputationMultiplier=1.f) const;
 UFUNCTION(BlueprintPure,Category="Askir|Merchant") int32 GetUnitSellPrice(FName ItemID,float ReputationMultiplier=1.f) const;
 UFUNCTION(BlueprintCallable,Category="Askir|Merchant") bool BuyFromMerchant(FName ItemID,int32 Quantity,UPARAM(ref) int32& PlayerGold,float ReputationMultiplier=1.f);
 UFUNCTION(BlueprintCallable,Category="Askir|Merchant") bool SellToMerchant(FName ItemID,int32 Quantity,UPARAM(ref) int32& PlayerGold,float ReputationMultiplier=1.f);
 UFUNCTION(BlueprintCallable,Category="Askir|Merchant") void Restock(int32 UnitsPerItem=1);
private:
 friend class UAskirTradeBridgeComponent;
 bool bTransactionInProgress=false;
};
