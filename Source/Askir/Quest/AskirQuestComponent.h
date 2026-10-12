#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest/AskirQuestData.h"
#include "AskirQuestComponent.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAskirQuestProgressChanged,FName,QuestID,FName,ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAskirQuestCompleted,FName,QuestID);
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirQuestComponent:public UActorComponent
{
 GENERATED_BODY()
public:
 UAskirQuestComponent();
 UFUNCTION(BlueprintCallable,Category="Askir|Quest") bool StartQuest(UAskirQuestData* Quest);
 UFUNCTION(BlueprintCallable,Category="Askir|Quest") bool AdvanceObjective(FName ObjectiveID,int32 Amount=1);
 UFUNCTION(BlueprintCallable,Category="Askir|Quest") bool AdvanceQuestObjective(FName QuestID,FName ObjectiveID,int32 Amount=1);
 UFUNCTION(BlueprintPure,Category="Askir|Quest") bool AreObjectivesComplete()const;
 UFUNCTION(BlueprintPure,Category="Askir|Quest") bool AreQuestObjectivesComplete(FName QuestID)const;
 UFUNCTION(BlueprintCallable,Category="Askir|Quest") bool CompleteQuest(UAskirQuestData* Quest);
 UFUNCTION(BlueprintPure,Category="Askir|Quest") EAskirQuestState GetState(UAskirQuestData* Quest)const;
 UFUNCTION(BlueprintPure,Category="Askir|Quest") EAskirQuestState GetStateByID(FName QuestID)const;
 UFUNCTION(BlueprintPure,Category="Askir|Quest") TArray<FAskirQuestObjective> GetObjectivesForQuest(FName QuestID)const;
 UPROPERTY(BlueprintAssignable,Category="Askir|Quest") FAskirQuestProgressChanged OnQuestProgressChanged;
 UPROPERTY(BlueprintAssignable,Category="Askir|Quest") FAskirQuestCompleted OnQuestCompleted;
 UFUNCTION(BlueprintCallable,Category="Askir|Quest") bool TurnInQuestWithRewards(UAskirQuestData* Quest);
 UFUNCTION(BlueprintPure,Category="Askir|Quest") bool HasClaimedReward(FName QuestID)const;
 /** Restore a validated authoritative quest snapshot, without granting rewards. */
 bool RestoreQuestProgress(FName QuestID,const TArray<FAskirQuestObjective>& Objectives,EAskirQuestState State,bool bRewardClaimed,bool bNotify=true);
 /** Call after restoring the persisted public fields. Repairs legacy mirrors and invalid entries. */
 UFUNCTION(BlueprintCallable,Category="Askir|Quest|Save") void NormalizeRestoredState(bool bNotify=true);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Quest") TSet<FName> ClaimedRewards;
 /** Persistent unique quest pickups; prevents overlap farming across save/load. */
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Quest") TSet<FName> ConsumedQuestTriggers;
 UFUNCTION(BlueprintPure,Category="Askir|Quest") bool HasConsumedQuestTrigger(FName TriggerID) const { return !TriggerID.IsNone() && ConsumedQuestTriggers.Contains(TriggerID); }
 /** Persistent one-shot encounter completion IDs; independent of quest rewards. */
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Encounter") TSet<FName> CompletedEncounters;
 UFUNCTION(BlueprintPure,Category="Askir|Encounter") bool HasCompletedEncounter(FName EncounterID) const { return !EncounterID.IsNone() && CompletedEncounters.Contains(EncounterID); }
 UFUNCTION(BlueprintCallable,Category="Askir|Encounter") bool MarkEncounterCompleted(FName EncounterID) { if(EncounterID.IsNone() || CompletedEncounters.Contains(EncounterID))return false; CompletedEncounters.Add(EncounterID); return true; }
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Quest") TMap<FName,EAskirQuestState> QuestStates;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Quest") TArray<FAskirQuestObjective> ActiveObjectives;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Quest") TMap<FName,FAskirQuestObjectiveList> ObjectivesByQuest;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Quest") FName LegacyActiveQuestID=NAME_None;
private:
 void RefreshLegacyObjectives();
 bool bRewardTransaction=false;
};
