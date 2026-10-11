#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirEquipmentComponent.generated.h"

UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirEquipmentComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 /** Stable item IDs; these must match the owning inventory before equipping. */
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Equipment") FName EquippedWeaponID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Equipment") FName EquippedArmorID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Equipment") float Durability=100.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Equipment") float ArmorRating=0.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Equipment") float WeaponRating=0.f;
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void EquipArmor(float Rating);
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void EquipWeapon(float Rating);
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void DamageGear(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void RepairGear(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void UnequipAll();
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void SetEquippedWeapon(FName ItemID,float Rating);
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void SetEquippedArmor(FName ItemID,float Rating);
 /** Equips only an item that the owner actually holds and that matches the requested slot. */
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") bool TryEquipInventoryItem(class UAskirItemData* Item);
 /** Removes stale equipment references after inventory changes or save restoration. */
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void ReconcileWithInventory();
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void RestoreEquipment(FName WeaponID,FName ArmorID,float Weapon,float Armor,float Condition);
};
