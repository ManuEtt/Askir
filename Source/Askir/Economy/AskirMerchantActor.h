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
 virtual void BeginPlay() override;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Mesh;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UAskirMerchantComponent> Merchant;
 UPROPERTY(BlueprintAssignable) FAskirMerchantOpened OnMerchantOpened;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float InteractionRange=250.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Merchant") FText MerchantName;
 UFUNCTION(BlueprintPure,Category="Askir|Merchant") bool CanInteract(AActor* Interactor) const;
 virtual bool Interact_Implementation(AActor* Interactor)override;
 virtual FText GetInteractionText_Implementation()const override;
};
