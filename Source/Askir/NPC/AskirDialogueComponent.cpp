#include "NPC/AskirDialogueComponent.h"
UAskirDialogueComponent::UAskirDialogueComponent(){PrimaryComponentTick.bCanEverTick=false;}
const FAskirDialogueNode* UAskirDialogueComponent::FindNode(FName ID) const
{for(const auto& N:Nodes)if(N.NodeID==ID)return &N;return nullptr;}
bool UAskirDialogueComponent::BeginDialogue()
{
 if(bActive||!FindNode(StartNode))return false;
 CurrentNode=StartNode;bActive=true;OnNodeChanged.Broadcast(CurrentNode);return true;
}
void UAskirDialogueComponent::EndDialogue(){bActive=false;CurrentNode=NAME_None;}
void UAskirDialogueComponent::AddFlag(FName Flag){if(!Flag.IsNone())Flags.AddUnique(Flag);}
bool UAskirDialogueComponent::HasFlag(FName Flag) const{return Flag.IsNone()||Flags.Contains(Flag);}
FAskirDialogueNode UAskirDialogueComponent::GetCurrentNode() const
{const auto* N=bActive?FindNode(CurrentNode):nullptr;return N?*N:FAskirDialogueNode();}
TArray<FAskirDialogueChoice> UAskirDialogueComponent::GetAvailableChoices() const
{
 TArray<FAskirDialogueChoice> Result;
 const auto* N=bActive?FindNode(CurrentNode):nullptr;
 if(N)for(const auto& C:N->Choices)if(HasFlag(C.RequiredFlag))Result.Add(C);
 return Result;
}
bool UAskirDialogueComponent::SelectChoice(int32 VisibleChoiceIndex)
{
 const auto Choices=GetAvailableChoices();
 if(!Choices.IsValidIndex(VisibleChoiceIndex))return false;
 const auto& Choice=Choices[VisibleChoiceIndex];
 if(!Choice.NextNode.IsNone()&&!FindNode(Choice.NextNode))return false;
 if(!Choice.GrantedFlag.IsNone())AddFlag(Choice.GrantedFlag);
 if(Choice.NextNode.IsNone()){EndDialogue();return true;}
 CurrentNode=Choice.NextNode;OnNodeChanged.Broadcast(CurrentNode);return true;
}

void UAskirDialogueComponent::RestoreFlags(const TArray<FName>& InFlags)
{
 Flags.Reset();
 for(FName Flag:InFlags)AddFlag(Flag);
}
