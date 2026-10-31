#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AskirInteractable.h"
#include "AskirHerbPatchActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UAskirQuestComponent;

/** The single authored SQ_003 exploration point: proximity alone grants nothing. */
UCLASS(Blueprintable)
class ASKIR_API AAskirHerbPatchActor : public AActor,public IAskirInteractable
{
    GENERATED_BODY()
public:
    AAskirHerbPatchActor();
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Herbs") TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Herbs") TObjectPtr<UStaticMeshComponent> Visual;
    /** Trace target independent of thin leaf geometry; never blocks walking/navigation. */
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Herbs") TObjectPtr<UBoxComponent> InteractionQuery;
    UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Herbs") FName SourceID=FName(TEXT("SQ003_Meadow_V84"));
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Herbs") float InteractionRange=240.f;
    UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Herbs") TObjectPtr<AActor> MissionPlayer;
    UFUNCTION(BlueprintPure,Category="Askir|Herbs") bool CanInteractWith(AActor* Interactor) const;
    UFUNCTION(BlueprintCallable,Category="Askir|Herbs|Save") bool BindMissionEvents();
    UFUNCTION(BlueprintCallable,Category="Askir|Herbs|Save") void RefreshMissionState();
    virtual bool Interact_Implementation(AActor* Interactor) override;
    virtual FText GetInteractionText_Implementation() const override;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UFUNCTION() void OnQuestProgressChanged(FName QuestID,FName ObjectiveID);
    void ClearMissionBinding();
    bool bMissionBindingEnded=false;
    UPROPERTY(Transient) TObjectPtr<UAskirQuestComponent> BoundJournal;
    FTimerHandle BindRetryTimer;
};
