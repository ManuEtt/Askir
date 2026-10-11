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
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Quest") TSet<FName> ClaimedRewards;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Quest") TMap<FName,EAskirQuestState> QuestStates;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Quest") TArray<FAskirQuestObjective> ActiveObjectives;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Quest") TMap<FName,FAskirQuestObjectiveList> ObjectivesByQuest;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Quest") FName LegacyActiveQuestID=NAME_None;
};
