#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirInventoryComponent.generated.h"
class UAskirItemData;
USTRUCT(BlueprintType)struct FAskirInventoryEntry
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
    UFUNCTION(BlueprintPure) bool HasItem(UAskirItemData* Item,int32 Quantity=1)const;
    UFUNCTION(BlueprintCallable) bool UseItem(UAskirItemData* Item);
    UFUNCTION(BlueprintPure) int32 GetItemCount(UAskirItemData* Item)const;
    UFUNCTION(BlueprintPure) float GetTotalWeight()const;
    UPROPERTY(BlueprintAssignable) FAskirInventoryChanged OnInventoryChanged;
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TArray<FAskirInventoryEntry> Items;
};