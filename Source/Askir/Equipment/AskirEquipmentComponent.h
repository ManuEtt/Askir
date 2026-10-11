#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirEquipmentComponent.generated.h"

UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirEquipmentComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Equipment") float Durability=100.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Equipment") float ArmorRating=0.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Equipment") float WeaponRating=0.f;
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void EquipArmor(float Rating);
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void EquipWeapon(float Rating);
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void DamageGear(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void RepairGear(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Equipment") void UnequipAll();
};
