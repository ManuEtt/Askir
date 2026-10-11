#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest/AskirQuestData.h"
#include "AskirQuestComponent.generated.h"
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))class ASKIR_API UAskirQuestComponent:public UActorComponent{GENERATED_BODY()public:UAskirQuestComponent();UFUNCTION(BlueprintCallable)bool StartQuest(UAskirQuestData*Q);UFUNCTION(BlueprintCallable)bool AdvanceObjective(FName ID,int32 Amount=1); UFUNCTION(BlueprintPure)bool AreObjectivesComplete()const;UFUNCTION(BlueprintCallable)bool CompleteQuest(UAskirQuestData*Q);UFUNCTION(BlueprintPure)EAskirQuestState GetState(UAskirQuestData*Q)const;UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)TMap<FName,EAskirQuestState> QuestStates;UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)TArray<FAskirQuestObjective> ActiveObjectives;};
