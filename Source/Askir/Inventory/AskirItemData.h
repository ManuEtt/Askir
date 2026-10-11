#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AskirItemData.generated.h"
UENUM(BlueprintType)enum class EAskirItemType:uint8{Misc,Consumable,Weapon,Armor,Quest};
UCLASS(BlueprintType)
class ASKIR_API UAskirItemData:public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) FName ItemID;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) FText Description;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) EAskirItemType ItemType=EAskirItemType::Misc;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) int32 MaxStackSize=1;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) float Weight=0;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) int32 Value=0;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) float Damage=0;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) float AttackRange=150;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) float AttackCooldown=.7;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) float HealAmount=0;
};