#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AskirInteractable.h"
#include "AskirDornfeldEventActor.generated.h"
UENUM(BlueprintType) enum class EAskirDornfeldEvent : uint8 { ElderDialogue, Shard, GuardianDefeated, ChestOpened };
/** Placeable event adapter: shard/elder can be interacted with; death/chest may call TriggerEvent in Blueprint. */
UCLASS(Blueprintable)
class ASKIR_API AAskirDornfeldEventActor : public AActor, public IAskirInteractable
{
 GENERATED_BODY()
public:
 AAskirDornfeldEventActor();
 UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Dornfeld") EAskirDornfeldEvent Event=EAskirDornfeldEvent::Shard;
 UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Dornfeld") FName PersistentEventID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Dornfeld") float InteractionRange=250.f;
 UFUNCTION(BlueprintCallable,Category="Askir|Dornfeld") bool TriggerEvent(AActor* Player);
 virtual bool Interact_Implementation(AActor* Interactor) override;
 virtual FText GetInteractionText_Implementation() const override;
};
