#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AskirInteractable.h"
#include "AskirCraftingStationActor.generated.h"
class UStaticMeshComponent;

UCLASS(Blueprintable)
class ASKIR_API AAskirCraftingStationActor : public AActor, public IAskirInteractable
{
    GENERATED_BODY()
public:
    AAskirCraftingStationActor();
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Crafting") FText StationName;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Crafting") float InteractionRange=250.f;
    UFUNCTION(BlueprintPure,Category="Askir|Crafting") bool CanInteract(AActor* Interactor) const;
    virtual bool Interact_Implementation(AActor* Interactor) override;
    virtual FText GetInteractionText_Implementation() const override;
};
