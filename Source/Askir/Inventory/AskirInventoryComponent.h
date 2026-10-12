#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirInventoryComponent.generated.h"

class UAskirItemData;
USTRUCT(BlueprintType)
struct FAskirInventoryEntry
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TObjectPtr<UAskirItemData> Item=nullptr;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) int32 Quantity=0;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAskirInventoryChanged);
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirInventoryComponent:public UActorComponent
{
    GENERATED_BODY()
public:
    UAskirInventoryComponent();
    UFUNCTION(BlueprintCallable) bool AddItem(UAskirItemData* Item,int32 Quantity=1);
    UFUNCTION(BlueprintCallable) bool RemoveItem(UAskirItemData* Item,int32 Quantity=1);
    // All requirements are checked against the original inventory before output is added.
    UFUNCTION(BlueprintCallable) bool ExchangeItems(const TArray<FAskirInventoryEntry>& RemovedItems,const TArray<FAskirInventoryEntry>& AddedItems);
    UFUNCTION(BlueprintPure) bool CanExchangeItems(const TArray<FAskirInventoryEntry>& RemovedItems,const TArray<FAskirInventoryEntry>& AddedItems) const;
    UFUNCTION(BlueprintPure) bool HasItem(UAskirItemData* Item,int32 Quantity=1)const;
    UFUNCTION(BlueprintCallable) bool UseItem(UAskirItemData* Item);
    UFUNCTION(BlueprintPure) int32 GetItemCount(UAskirItemData* Item)const;
    UFUNCTION(BlueprintPure) float GetTotalWeight()const;
    UPROPERTY(BlueprintAssignable) FAskirInventoryChanged OnInventoryChanged;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TArray<FAskirInventoryEntry> Items;
    // Prevent corrupted quantities or tiny stacks from allocating an unbounded array.
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Askir|Inventory",meta=(ClampMin="1",ClampMax="4096")) int32 MaxInventorySlots=4096;
private:
    friend class UAskirTradeBridgeComponent;
    friend class UAskirCraftingComponent;
    friend class UAskirQuestComponent;
    friend class AAskirLootPickup;
    friend class AAskirLootChest;
    bool PrepareExchange(const TArray<FAskirInventoryEntry>& RemovedItems,const TArray<FAskirInventoryEntry>& AddedItems,TArray<FAskirInventoryEntry>& PreparedItems) const;
    bool bTransactionInProgress=false;
};
