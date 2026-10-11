#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AskirProductionRows.generated.h"

USTRUCT(BlueprintType)
struct FAskirProductionQuestRow : public FTableRowBase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Title;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Description;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ObjectiveId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 RequiredCount = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ExperienceReward = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 GoldReward = 0;
};

USTRUCT(BlueprintType)
struct FAskirProductionSpellRow : public FTableRowBase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Name;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName Element;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ManaCost = 10.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CooldownSeconds = 2.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Power = 10.f;
};

USTRUCT(BlueprintType)
struct FAskirProductionItemRow : public FTableRowBase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Name;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Description;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName Category;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Value = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 StackMax = 1;
};
