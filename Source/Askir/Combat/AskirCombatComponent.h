#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/AskirDamageTypes.h"
#include "AskirCombatComponent.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAskirDamageTaken,const FAskirDamageInfo&,DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAskirDeath);
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirCombatComponent:public UActorComponent
{
    GENERATED_BODY()
public:
    UAskirCombatComponent();
    UFUNCTION(BlueprintCallable) float ApplyDamage(const FAskirDamageInfo& Info);
    UFUNCTION(BlueprintCallable) void Heal(float Amount);
    UFUNCTION(BlueprintCallable) void Kill();
    UFUNCTION(BlueprintCallable) void ResetHealth();
    UFUNCTION(BlueprintCallable) void RestoreHealth(float SavedHealth);
    UFUNCTION(BlueprintPure) float GetHealth()const{return Health;}
    UFUNCTION(BlueprintPure) float GetMaxHealth()const{return MaxHealth;}
    UFUNCTION(BlueprintPure) float GetHealthPercent()const;
    UFUNCTION(BlueprintPure) bool IsDead()const{return bDead;}
    UPROPERTY(BlueprintAssignable) FAskirDamageTaken OnDamageTaken;
    UPROPERTY(BlueprintAssignable) FAskirDeath OnDeath;
protected:
    UPROPERTY(EditAnywhere) float MaxHealth=100;
    UPROPERTY(VisibleInstanceOnly) float Health=100;
    UPROPERTY(EditAnywhere) float PhysicalResistance=0;
    UPROPERTY(EditAnywhere) float MagicResistance=0;
private:
    bool bDead=false;
};