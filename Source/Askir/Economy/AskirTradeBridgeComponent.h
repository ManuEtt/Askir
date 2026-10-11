#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
 UFUNCTION(BlueprintCallable,Category="Askir|Trade") bool BuyItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32& PlayerGold,float PriceMultiplier=1.f);
 UFUNCTION(BlueprintCallable,Category="Askir|Trade") bool SellItem(UAskirMerchantComponent* Merchant,UAskirInventoryComponent* PlayerInventory,FName ItemID,int32 Quantity,int32& PlayerGold,float PriceMultiplier=1.f);
private:
 UAskirItemData* FindItem(FName ItemID) const;
};
