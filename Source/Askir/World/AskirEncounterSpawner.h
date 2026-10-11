#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AskirEncounterSpawner.generated.h"
class AAskirEncounterZone;
class UAskirCombatComponent;
class AAskirEncounterSpawner;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAskirWaveFinished,AActor*,Player,FName,EncounterID);
/** One enemy instance tracks its own death and reports to the encounter spawner. */
UCLASS(ClassGroup=(Askir),meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirEncounterEnemyTracker : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(Transient) TObjectPtr<AAskirEncounterSpawner> Spawner;
 UPROPERTY(Transient) TObjectPtr<UAskirCombatComponent> Combat;
 UFUNCTION() void HandleDeath();
};
/** Optional runtime enemy waves driven by AAskirEncounterZone. Configure in editor. */
UCLASS(BlueprintType)
class ASKIR_API AAskirEncounterSpawner : public AActor
{
 GENERATED_BODY()
public:
 AAskirEncounterSpawner();
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Encounter") TObjectPtr<AAskirEncounterZone> Zone;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Encounter") TSubclassOf<APawn> EnemyClass;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Encounter",meta=(ClampMin="1",ClampMax="32")) int32 EnemiesPerWave=3;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Encounter",meta=(ClampMin="1",ClampMax="10")) int32 WaveCount=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Encounter",meta=(ClampMin="0")) float SpawnRadius=350.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Encounter") FName CompletionQuestID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Encounter") FName CompletionObjectiveID=NAME_None;
 UPROPERTY(BlueprintAssignable,Category="Askir|Encounter") FAskirWaveFinished OnEncounterFinished;
 UFUNCTION(BlueprintCallable,Category="Askir|Encounter") bool StartForPlayer(AActor* Player,FName EncounterID);
 UFUNCTION(BlueprintCallable,Category="Askir|Encounter") void ResetSpawner();
 UFUNCTION() void HandleEncounterTriggered(AActor* Player,FName EncounterID);
 void NotifyEnemyDefeated();
protected:
 virtual void BeginPlay() override;
private:
 bool SpawnWave();
 UPROPERTY(Transient) TObjectPtr<AActor> ActivePlayer;
 UPROPERTY(Transient) TArray<TObjectPtr<APawn>> LivingEnemies;
 int32 WavesStarted=0;
 bool bRunning=false;
 bool bFinished=false;
};
