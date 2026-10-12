#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AskirQuestData.generated.h"

UENUM(BlueprintType)
enum class EAskirQuestState : uint8 { Inactive, Available, Active, Completed, Failed };
USTRUCT(BlueprintType)
struct FAskirQuestObjective
{
 GENERATED_BODY()
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,SaveGame) FName ObjectiveID;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,SaveGame) FText Description;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,SaveGame) int32 RequiredCount=1;
 UPROPERTY(BlueprintReadOnly,SaveGame) int32 CurrentCount=0;
};
USTRUCT(BlueprintType)
struct FAskirQuestObjectiveList
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) TArray<FAskirQuestObjective> Objectives;
};
UCLASS(BlueprintType)
class ASKIR_API UAskirQuestData : public UPrimaryDataAsset
{
 GENERATED_BODY()
public:
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) FName QuestID;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) FText Title;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) FText Description;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) TArray<FAskirQuestObjective> Objectives;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) int32 GoldReward=0;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) int32 ExperienceReward=0;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) TObjectPtr<class UAskirItemData> RewardItem=nullptr;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly) int32 RewardItemQuantity=0;
};
