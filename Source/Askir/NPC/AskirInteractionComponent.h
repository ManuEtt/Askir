#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirInteractionComponent.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAskirInteracted,AActor*,Interactor);
UCLASS(ClassGroup=(Askir),meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirInteractionComponent:public UActorComponent
{
 GENERATED_BODY()
public:
 UAskirInteractionComponent();
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float MaxInteractionDistance=250.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FText Prompt=FText::FromString(TEXT("Interagieren"));
 UPROPERTY(BlueprintAssignable) FAskirInteracted OnInteracted;
 UFUNCTION(BlueprintCallable) bool Interact(AActor* Interactor);
 UFUNCTION(BlueprintPure) bool CanInteract(AActor* Interactor) const;
};
