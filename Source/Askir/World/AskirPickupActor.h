#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AskirInteractable.h"
#include "AskirPickupActor.generated.h"
class UStaticMeshComponent;class UAskirItemData;
UCLASS()
class ASKIR_API AAskirPickupActor:public AActor,public IAskirInteractable
{
    GENERATED_BODY()
public:
    AAskirPickupActor();
    virtual bool Interact_Implementation(AActor*)override;
    virtual FText GetInteractionText_Implementation()const override;
    UPROPERTY(VisibleAnywhere)TObjectPtr<UStaticMeshComponent>Mesh;
    UPROPERTY(EditAnywhere,BlueprintReadWrite)TObjectPtr<UAskirItemData>ItemData;
    UPROPERTY(EditAnywhere,BlueprintReadWrite)int32 Quantity=1;
};