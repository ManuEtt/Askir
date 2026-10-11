#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirInteractionComponent.generated.h"
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirInteractionComponent:public UActorComponent
{
    GENERATED_BODY()
public:
    UAskirInteractionComponent();
    UFUNCTION(BlueprintCallable)bool TryInteract(float Distance=250);
    UPROPERTY(EditAnywhere)float TraceRadius=12;
};