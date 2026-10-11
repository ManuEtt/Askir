#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AskirDornfeldMissionRelay.generated.h"
class AAskirLootPickup;
class AAskirLootChest;
class UAskirCombatComponent;
/** Editor-placeable adapter connecting real inventory/loot/death events to the Dornfeld mission. */
UCLASS(Blueprintable)
class ASKIR_API AAskirDornfeldMissionRelay : public AActor
{
 GENERATED_BODY()
public:
 AAskirDornfeldMissionRelay();
 /** The quest player; when empty, the first local player pawn is resolved on BeginPlay. */
 UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Mission") TObjectPtr<AActor> MissionPlayer;
 UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Mission") TObjectPtr<AAskirLootPickup> ShardPickup;
 UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Mission") TObjectPtr<AAskirLootChest> MissionChest;
 UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Mission") TObjectPtr<AActor> GuardianActor;
 /** Stable mission ID, unique for each shard/guardian/chest. */
 UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Askir|Mission") FName EventID=NAME_None;
 /** Retry binding while the player pawn is being spawned or possessed. */
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Mission|Reliability",meta=(ClampMin="0.1")) float BindRetryInterval=0.5f;
 UFUNCTION(BlueprintCallable,Category="Askir|Mission") bool BindMissionEvents();
 UFUNCTION(BlueprintPure,Category="Askir|Mission") bool IsBound() const {return bBound;}
protected:
 virtual void BeginPlay() override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UFUNCTION() void OnShardCollected(AActor* Collector);
 UFUNCTION() void OnChestOpened(AActor* Opener);
 UFUNCTION() void OnGuardianDied();
private:
 UPROPERTY(Transient) TObjectPtr<UAskirCombatComponent> GuardianCombat;
 FTimerHandle BindRetryTimer;
 void RetryBindMissionEvents();
 bool bBound=false;
};
