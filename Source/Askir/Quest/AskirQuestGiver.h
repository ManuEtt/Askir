#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AskirInteractable.h"
#include "AskirQuestGiver.generated.h"
class UStaticMeshComponent;class UAskirQuestData;
UCLASS(Blueprintable)
class ASKIR_API AAskirQuestGiver:public AActor,public IAskirInteractable
{
 GENERATED_BODY()
public:
 AAskirQuestGiver();
 virtual bool Interact_Implementation(AActor* Interactor)override;
 virtual FText GetInteractionText_Implementation()const override;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Quest")TObjectPtr<UStaticMeshComponent> Mesh;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Quest")TObjectPtr<UAskirQuestData> Quest;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Quest")bool bAllowTurnIn=true;
};
