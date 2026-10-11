#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AskirInteractable.h"
#include "AskirMerchantActor.generated.h"
class UStaticMeshComponent;class UAskirMerchantComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAskirMerchantOpened,AActor*,Customer);
UCLASS(Blueprintable)
class ASKIR_API AAskirMerchantActor:public AActor,public IAskirInteractable
{
 GENERATED_BODY()
public:
 AAskirMerchantActor();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Mesh;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UAskirMerchantComponent> Merchant;
 UPROPERTY(BlueprintAssignable) FAskirMerchantOpened OnMerchantOpened;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float InteractionRange=250.f;
 virtual bool Interact_Implementation(AActor* Interactor)override;
 virtual FText GetInteractionText_Implementation()const override;
};
