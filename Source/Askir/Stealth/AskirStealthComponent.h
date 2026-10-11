#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirStealthComponent.generated.h"

UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirStealthComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Stealth") float Noise=0.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Stealth") float Visibility=100.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Stealth") float Suspicion=0.f;
 UFUNCTION(BlueprintCallable,Category="Askir|Stealth") void MakeNoise(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Stealth") void Crouch();
 UFUNCTION(BlueprintCallable,Category="Askir|Stealth") void StandUp();
 UFUNCTION(BlueprintCallable,Category="Askir|Stealth") void RaiseSuspicion(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Stealth") void CalmDown(float Amount);
};
