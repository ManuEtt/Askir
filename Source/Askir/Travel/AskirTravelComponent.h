#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirTravelComponent.generated.h"

UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirTravelComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Travel") float TravelDistance=0.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Travel") float TravelFatigue=0.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Travel") float Supplies=100.f;
 UFUNCTION(BlueprintCallable,Category="Askir|Travel") void Walk(float Distance);
 UFUNCTION(BlueprintCallable,Category="Askir|Travel") void ConsumeSupplies(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Travel") void Rest(float Hours);
 UFUNCTION(BlueprintCallable,Category="Askir|Travel") void Resupply(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Travel") void FastTravel(float Distance);
};
