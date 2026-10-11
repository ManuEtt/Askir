#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirCampComponent.generated.h"

UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirCampComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Camp") float Fuel=0.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Camp") float Food=0.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Camp") float Shelter=0.f;
 UFUNCTION(BlueprintCallable,Category="Askir|Camp") void GatherFuel(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Camp") void StockFood(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Camp") void BuildShelter(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Camp") void BurnFuel(float Hours);
 UFUNCTION(BlueprintCallable,Category="Askir|Camp") void EatRations(float Amount);
};
