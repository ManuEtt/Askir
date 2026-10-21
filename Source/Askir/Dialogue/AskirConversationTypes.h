#pragma once
#include "CoreMinimal.h"
#include "AskirConversationTypes.generated.h"

USTRUCT(BlueprintType)
struct FAskirConversationChoice
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FName ChoiceID=NAME_None;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FText Label;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FName NextNode=NAME_None;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FName> RequiredFlags;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FName> ForbiddenFlags;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FName GrantedFlag=NAME_None;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FName Action=NAME_None;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FName Payload=NAME_None;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bEnabled=true;
};
USTRUCT(BlueprintType)
struct FAskirConversationNode
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FName NodeID=NAME_None;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FText Speaker;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FText Text;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FAskirConversationChoice> Choices;
};
USTRUCT(BlueprintType)
struct FAskirConversationCondition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 DeliveryStage=-1;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FName> RequiredFlags;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FName> ForbiddenFlags;
};
USTRUCT(BlueprintType)
struct FAskirConversationEntry
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FAskirConversationCondition Condition;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FName NodeID=NAME_None;
};
USTRUCT(BlueprintType)
struct FAskirConversationGraph
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FName ID=NAME_None;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FName StartNode=NAME_None;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FAskirConversationEntry> Entries;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FAskirConversationNode> Nodes;
};
