#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirSurvivalComponent.generated.h"

UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirSurvivalComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Survival") float Warmth=50.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Survival") float Wetness=0.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Survival") float Exposure=0.f;
 UFUNCTION(BlueprintCallable,Category="Askir|Survival") void AddWarmth(float Value);
 UFUNCTION(BlueprintCallable,Category="Askir|Survival") void AddWetness(float Value);
 UFUNCTION(BlueprintCallable,Category="Askir|Survival") void ApplyExposure(float Hours);
 UFUNCTION(BlueprintCallable,Category="Askir|Survival") void DryOff(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Survival") void RestAtFire(float Hours);
};
