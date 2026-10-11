#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AskirEncounterZone.generated.h"
class UBoxComponent;
class UPrimitiveComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAskirEncounterTriggered,AActor*,Player,FName,EncounterID);
/** Authorable village encounter: player enters a zone, quest objective advances, Blueprint handles presentation/spawns. */
UCLASS(BlueprintType)
class ASKIR_API AAskirEncounterZone : public AActor
{
 GENERATED_BODY()
public:
 AAskirEncounterZone();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Encounter") TObjectPtr<UBoxComponent> Trigger;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Encounter") FName EncounterID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Encounter") FName QuestID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Encounter") FName ObjectiveID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Encounter",meta=(ClampMin="1")) int32 ObjectiveAmount=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Encounter") bool bOncePerPlayer=true;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Encounter") bool bRequireActiveQuest=false;
 UPROPERTY(BlueprintAssignable,Category="Askir|Encounter") FAskirEncounterTriggered OnEncounterTriggered;
 UFUNCTION(BlueprintCallable,Category="Askir|Encounter") bool ActivateForPlayer(AActor* Player);
 UFUNCTION(BlueprintCallable,Category="Askir|Encounter") void ResetEncounter();
protected:
 UFUNCTION() void HandleOverlap(UPrimitiveComponent* Overlapped,AActor* Other,UPrimitiveComponent* OtherComp,int32 BodyIndex,bool bFromSweep,const FHitResult& Hit);
private:
 UPROPERTY(Transient) TSet<TObjectPtr<AActor>> ActivatedPlayers;
};
