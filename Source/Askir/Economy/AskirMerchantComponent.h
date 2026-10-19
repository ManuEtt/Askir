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
/** Stable values only: prices and item assets remain authored catalog data. */
USTRUCT(BlueprintType)
struct FAskirMerchantSnapshot
{
 GENERATED_BODY()
 UPROPERTY(SaveGame,BlueprintReadOnly) int32 Gold=0;
 UPROPERTY(SaveGame,BlueprintReadOnly) TMap<FName,int32> StockByItem;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAskirShopTrade,FName,ItemID,int32,Quantity,int32,TotalGold);
UCLASS(ClassGroup=(Askir),meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirMerchantComponent:public UActorComponent
{
 GENERATED_BODY()
public:
 UAskirMerchantComponent();
 virtual void BeginPlay() override;
 /** Required and unique within a map for persistent merchants. */
 UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Askir|Merchant") FName MerchantID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Merchant") TArray<FAskirShopItem> Inventory;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Merchant") int32 MerchantGold=500;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Merchant") float SellBackRatio=.5f;
 UPROPERTY(BlueprintAssignable,Category="Askir|Merchant") FAskirShopTrade OnTradeCompleted;
 UFUNCTION(BlueprintPure,Category="Askir|Merchant") int32 GetUnitBuyPrice(FName ItemID,float ReputationMultiplier=1.f) const;
 UFUNCTION(BlueprintPure,Category="Askir|Merchant") int32 GetUnitSellPrice(FName ItemID,float ReputationMultiplier=1.f) const;
 UFUNCTION(BlueprintCallable,Category="Askir|Merchant") bool BuyFromMerchant(FName ItemID,int32 Quantity,UPARAM(ref) int32& PlayerGold,float ReputationMultiplier=1.f);
 UFUNCTION(BlueprintCallable,Category="Askir|Merchant") bool SellToMerchant(FName ItemID,int32 Quantity,UPARAM(ref) int32& PlayerGold,float ReputationMultiplier=1.f);
 UFUNCTION(BlueprintCallable,Category="Askir|Merchant") void Restock(int32 UnitsPerItem=1);
 bool IsTransactionInProgress() const {return bTransactionInProgress;}
 bool CanAcceptTransactions() const;
 bool OwnsLedgerReference(const int32& Value) const;
 bool CaptureInitialState();
 bool GetInitialSnapshot(FAskirMerchantSnapshot& OutSnapshot) const;
 bool CaptureSnapshot(FAskirMerchantSnapshot& OutSnapshot) const;
 static bool ValidateSnapshotData(const FAskirMerchantSnapshot& Snapshot);
 bool CanRestoreSnapshot(const FAskirMerchantSnapshot& Snapshot) const;
 /** Installs validated state without firing a trade or replaying transactions. */
 bool RestoreSnapshot(const FAskirMerchantSnapshot& Snapshot);
private:
 friend class UAskirTradeBridgeComponent;
 bool bTransactionInProgress=false;
 bool bHasInitialSnapshot=false;
 UPROPERTY() FAskirMerchantSnapshot InitialSnapshot;
 bool HasValidCatalog() const;
};
