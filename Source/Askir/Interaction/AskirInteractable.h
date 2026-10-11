#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AskirInteractable.generated.h"
UINTERFACE(Blueprintable)
class ASKIR_API UAskirInteractable:public UInterface{GENERATED_BODY()};
class ASKIR_API IAskirInteractable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent,BlueprintCallable)bool Interact(AActor*Interactor);
    UFUNCTION(BlueprintNativeEvent,BlueprintCallable)FText GetInteractionText()const;
};