#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Economy/AskirEconomyRules.h"
#include "Inventory/AskirInventoryComponent.h"
#include "AskirTradeBridgeComponent.generated.h"
class UAskirInventoryComponent;
class UAskirItemData;
class UAskirMerchantComponent;
USTRUCT(BlueprintType)
struct FAskirTradeCatalogEntry
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName ItemID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TObjectPtr<UAskirItemData> Item=nullptr;
};
UCLASS(ClassGroup=(Askir),meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirTradeBridgeComponent:public UActorComponent
{
 GENERATED_BODY()
public:
 UAskirTradeBridgeComponent();
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Trade") TArray<FAskirTradeCatalogEntry> Catalog;
 UFUNCTION(BlueprintPure,Category="Askir|Trade") UAskirItemData* FindItem(FName ItemID) const;
 UFUNCTION(BlueprintPure,Category="Askir|Trade") bool CanBuyItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32 PlayerGold,float PriceMultiplier=1.f) const;
 UFUNCTION(BlueprintPure,Category="Askir|Trade") bool CanSellItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32 PlayerGold,float PriceMultiplier=1.f) const;
 UFUNCTION(BlueprintCallable,Category="Askir|Trade") bool BuyItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,UPARAM(ref) int32& PlayerGold,float PriceMultiplier=1.f);
 UFUNCTION(BlueprintCallable,Category="Askir|Trade") bool SellItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,UPARAM(ref) int32& PlayerGold,float PriceMultiplier=1.f);
private:
 bool PrepareTrade(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32 PlayerGold,float PriceMultiplier,AskirEconomyRules::Direction Operation,AskirEconomyRules::TradePlan& Plan,int32& StockIndex,TArray<FAskirInventoryEntry>& PreparedInventory) const;
 bool bTradeInProgress=false;
};
