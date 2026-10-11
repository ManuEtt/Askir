#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirCompanionComponent.generated.h"

UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirCompanionComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Companion") float Loyalty=50.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Companion") float Bond=0.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Companion") float Stress=0.f;
 UFUNCTION(BlueprintCallable,Category="Askir|Companion") void Praise(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Companion") void TrainTogether(float Hours);
 UFUNCTION(BlueprintCallable,Category="Askir|Companion") void IncreaseStress(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Companion") void Comfort(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Companion") void LoseLoyalty(float Amount);
};
