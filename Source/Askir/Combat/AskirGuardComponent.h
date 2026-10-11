#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirGuardComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAskirGuardBroken);
UCLASS(ClassGroup=(Askir), BlueprintType, meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirGuardComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UAskirGuardComponent();
 virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
 UFUNCTION(BlueprintCallable, Category="Askir|Guard") bool StartGuard();
 UFUNCTION(BlueprintCallable, Category="Askir|Guard") void StopGuard();
 UFUNCTION(BlueprintCallable, Category="Askir|Guard") float AbsorbIncomingDamage(float IncomingDamage);
 UFUNCTION(BlueprintCallable, Category="Askir|Guard") void RestoreStamina();
 UFUNCTION(BlueprintPure, Category="Askir|Guard") float GetStaminaPercent() const;
 UFUNCTION(BlueprintPure, Category="Askir|Guard") bool IsGuarding() const {return bGuarding;}
 UPROPERTY(BlueprintAssignable, Category="Askir|Guard") FAskirGuardBroken OnGuardBroken;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Guard", meta=(ClampMin="1")) float MaxStamina=100.f;
 UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Askir|Guard") float Stamina=100.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Guard", meta=(ClampMin="0")) float RegenPerSecond=14.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Guard", meta=(ClampMin="0", ClampMax="1")) float DamageReduction=0.65f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Guard", meta=(ClampMin="0")) float StaminaCostPerDamage=0.8f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Guard", meta=(ClampMin="0")) float GuardBreakCooldown=1.5f;
private:
 bool bGuarding=false;
 float GuardLockUntil=0.f;
};
