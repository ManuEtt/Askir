#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirMoraleComponent.generated.h"

UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirMoraleComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Morale") float Morale=50.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Morale") float Fear=0.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Morale") float Courage=50.f;
 UFUNCTION(BlueprintCallable,Category="Askir|Morale") void Encourage(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Morale") void Intimidate(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Morale") void RecoverFear(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Morale") void TrainCourage(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Morale") void BreakMorale(float Amount);
};
