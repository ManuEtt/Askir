#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AskirInteractable.h"
#include "AskirDeliveryQuestActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UAskirQuestComponent;
UENUM(BlueprintType)
enum class EAskirDeliveryQuestRole : uint8 { Giver, Cargo };

/** Real traceable Torben/cargo interaction. Merely approaching never completes SQ_001. */
UCLASS(Blueprintable)
class ASKIR_API AAskirDeliveryQuestActor : public AActor,public IAskirInteractable
{
    GENERATED_BODY()
public:
    AAskirDeliveryQuestActor();
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Delivery") TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Delivery") TObjectPtr<UStaticMeshComponent> Visual;
    UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Delivery") EAskirDeliveryQuestRole Role=EAskirDeliveryQuestRole::Giver;
    UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Delivery") FName EventID=NAME_None;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Delivery") FText DisplayName;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Delivery") float InteractionRange=250.f;
    UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Delivery") TObjectPtr<AActor> MissionPlayer;
    UFUNCTION(BlueprintPure,Category="Askir|Delivery") bool CanInteractWith(AActor* Interactor) const;
    UFUNCTION(BlueprintCallable,Category="Askir|Delivery|Save") bool BindMissionEvents();
    UFUNCTION(BlueprintCallable,Category="Askir|Delivery|Save") void RefreshMissionState();
    virtual bool Interact_Implementation(AActor* Interactor) override;
    virtual FText GetInteractionText_Implementation() const override;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UFUNCTION() void OnQuestProgressChanged(FName QuestID,FName ObjectiveID);
    UPROPERTY(Transient) TObjectPtr<UAskirQuestComponent> BoundJournal;
    FTimerHandle BindRetryTimer;
};
