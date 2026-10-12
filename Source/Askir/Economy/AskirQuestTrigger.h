#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AskirQuestTrigger.generated.h"
class UBoxComponent;class UPrimitiveComponent;
UCLASS(Blueprintable)
class ASKIR_API AAskirQuestTrigger:public AActor
{
 GENERATED_BODY()
public:
 AAskirQuestTrigger();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UBoxComponent> Trigger;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName QuestID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName ObjectiveID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Amount=1;
 /** Assign a unique stable ID to each physical pickup to prevent repeat collection after loading. */
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName PersistentTriggerID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bOneShotPerActor=true;
 UPROPERTY(Transient) TSet<TWeakObjectPtr<AActor>> TriggeredActors;
 UFUNCTION() void OnEnter(UPrimitiveComponent* Overlapped,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);
};
