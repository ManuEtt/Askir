#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AskirInteractable.h"
#include "AskirDornfeldEventActor.generated.h"
class UStaticMeshComponent;
UENUM(BlueprintType) enum class EAskirDornfeldEvent : uint8 { ElderDialogue, Shard, GuardianDefeated, ChestOpened };
/** Placeable Mara/shard adapter. Guardian/chest events require their real gameplay sources. */
UCLASS(Blueprintable)
class ASKIR_API AAskirDornfeldEventActor : public AActor, public IAskirInteractable
{
 GENERATED_BODY()
public:
 AAskirDornfeldEventActor();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Dornfeld") TObjectPtr<UStaticMeshComponent> Visual;
 UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Dornfeld") EAskirDornfeldEvent Event=EAskirDornfeldEvent::Shard;
 UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Dornfeld") FName PersistentEventID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Dornfeld") float InteractionRange=250.f;
 UFUNCTION(BlueprintCallable,Category="Askir|Dornfeld") bool TriggerEvent(AActor* Player);
 virtual bool Interact_Implementation(AActor* Interactor) override;
 virtual FText GetInteractionText_Implementation() const override;
};
