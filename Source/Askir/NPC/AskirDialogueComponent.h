#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirDialogueComponent.generated.h"
USTRUCT(BlueprintType)
struct FAskirDialogueChoice
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FText Label;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName NextNode=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName RequiredFlag=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName GrantedFlag=NAME_None;
};
USTRUCT(BlueprintType)
struct FAskirDialogueNode
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName NodeID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FText Speaker;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FText Text;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FAskirDialogueChoice> Choices;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAskirDialogueNodeChanged,FName,NodeID);
UCLASS(ClassGroup=(Askir),meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirDialogueComponent:public UActorComponent
{
 GENERATED_BODY()
public:
 UAskirDialogueComponent();
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Dialogue") TArray<FAskirDialogueNode> Nodes;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Dialogue") FName StartNode=FName(TEXT("Start"));
 UPROPERTY(BlueprintAssignable,Category="Askir|Dialogue") FAskirDialogueNodeChanged OnNodeChanged;
 UFUNCTION(BlueprintCallable,Category="Askir|Dialogue") bool BeginDialogue();
 UFUNCTION(BlueprintCallable,Category="Askir|Dialogue") bool SelectChoice(int32 VisibleChoiceIndex);
 UFUNCTION(BlueprintCallable,Category="Askir|Dialogue") void EndDialogue();
 UFUNCTION(BlueprintCallable,Category="Askir|Dialogue") void AddFlag(FName Flag);
 UFUNCTION(BlueprintPure,Category="Askir|Dialogue") bool HasFlag(FName Flag) const;
 UFUNCTION(BlueprintPure,Category="Askir|Dialogue") FAskirDialogueNode GetCurrentNode() const;
 UFUNCTION(BlueprintCallable,Category="Askir|Dialogue") TArray<FAskirDialogueChoice> GetAvailableChoices() const;
 UFUNCTION(BlueprintPure,Category="Askir|Dialogue") TArray<FName> GetFlags() const{return Flags;}
 UFUNCTION(BlueprintCallable,Category="Askir|Dialogue") void RestoreFlags(const TArray<FName>& InFlags);
 UFUNCTION(BlueprintPure,Category="Askir|Dialogue") bool IsActive() const{return bActive;}
private:
 UPROPERTY() TArray<FName> Flags;
 FName CurrentNode=NAME_None;
 bool bActive=false;
 const FAskirDialogueNode* FindNode(FName ID) const;
};
