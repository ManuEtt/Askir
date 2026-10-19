#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AskirDeliveryBanditRelay.generated.h"

class UAskirCombatComponent;
class UAskirDeliveryMissionComponent;
class UAskirQuestComponent;

/** Unique, player-attributed bandit death receipt; placed actors survive earlier loads. */
UCLASS(Blueprintable)
class ASKIR_API AAskirDeliveryBanditRelay : public AActor
{
    GENERATED_BODY()
public:
    AAskirDeliveryBanditRelay();
    UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Delivery") TObjectPtr<AActor> MissionPlayer;
    UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Delivery") TObjectPtr<AActor> BanditActor;
    UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Delivery") FName EventID=NAME_None;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Delivery",meta=(ClampMin="0.1")) float BindRetryInterval=.5f;
    UFUNCTION(BlueprintCallable,Category="Askir|Delivery") bool BindMissionEvents();
    UFUNCTION(BlueprintCallable,Category="Askir|Delivery|Save") void RefreshMissionState();
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UFUNCTION() void OnBanditDied();
    UFUNCTION() void OnQuestProgressChanged(FName QuestID,FName ObjectiveID);
    void RetryBind();
    void ReconcileState(bool bForceRestore);
    UPROPERTY(Transient) TObjectPtr<UAskirCombatComponent> BanditCombat;
    UPROPERTY(Transient) TObjectPtr<UAskirDeliveryMissionComponent> BoundMission;
    UPROPERTY(Transient) TObjectPtr<UAskirQuestComponent> BoundJournal;
    FTimerHandle BindRetryTimer;
    bool bBound=false;
    bool bInitialized=false;
    bool bLastDefeated=false;
    bool bLastActive=false;
};
