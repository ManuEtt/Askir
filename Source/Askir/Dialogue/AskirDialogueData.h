#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AskirDialogueData.generated.h"
USTRUCT(BlueprintType)struct FAskirDialogueCondition{GENERATED_BODY()UPROPERTY(EditAnywhere,BlueprintReadOnly)FName Key;UPROPERTY(EditAnywhere,BlueprintReadOnly)float MinimumValue=0;};
USTRUCT(BlueprintType)struct FAskirDialogueLine{GENERATED_BODY()UPROPERTY(EditAnywhere,BlueprintReadOnly)FName SpeakerTag;UPROPERTY(EditAnywhere,BlueprintReadOnly)FText Text;UPROPERTY(EditAnywhere,BlueprintReadOnly)TArray<FAskirDialogueCondition>Conditions;};
UCLASS(BlueprintType)class ASKIR_API UAskirDialogueData:public UPrimaryDataAsset{GENERATED_BODY()public:UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)FName DialogueID;UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)TArray<FAskirDialogueLine>Lines;};
