#include "Quest/AskirQuestComponent.h"
UAskirQuestComponent::UAskirQuestComponent(){PrimaryComponentTick.bCanEverTick=false;}
bool UAskirQuestComponent::StartQuest(UAskirQuestData* Quest)
{
 if(!IsValid(Quest)||Quest->QuestID.IsNone()||Quest->Objectives.IsEmpty())return false;
 if(GetStateByID(Quest->QuestID)==EAskirQuestState::Active||GetStateByID(Quest->QuestID)==EAskirQuestState::Completed)return false;
 TSet<FName> Seen;
 for(const auto& Objective:Quest->Objectives){if(Objective.ObjectiveID.IsNone()||Objective.RequiredCount<=0||Seen.Contains(Objective.ObjectiveID))return false;Seen.Add(Objective.ObjectiveID);}
 FAskirQuestObjectiveList List;List.Objectives=Quest->Objectives;
 for(auto& Objective:List.Objectives)Objective.CurrentCount=0;
 ObjectivesByQuest.Add(Quest->QuestID,List);
 QuestStates.Add(Quest->QuestID,EAskirQuestState::Active);
 if(LegacyActiveQuestID.IsNone()){LegacyActiveQuestID=Quest->QuestID;ActiveObjectives=List.Objectives;}
 return true;
}
bool UAskirQuestComponent::AdvanceQuestObjective(FName QuestID,FName ObjectiveID,int32 Amount)
{
 if(QuestID.IsNone()||ObjectiveID.IsNone()||Amount<=0||GetStateByID(QuestID)!=EAskirQuestState::Active)return false;
 FAskirQuestObjectiveList* List=ObjectivesByQuest.Find(QuestID);if(!List)return false;
 for(auto& Objective:List->Objectives)
 {
  if(Objective.ObjectiveID!=ObjectiveID||Objective.CurrentCount>=Objective.RequiredCount)continue;
  const int64 Next=int64(Objective.CurrentCount)+int64(Amount);
  Objective.CurrentCount=int32(FMath::Min<int64>(Objective.RequiredCount,Next));
  if(LegacyActiveQuestID==QuestID)ActiveObjectives=List->Objectives;
  OnQuestProgressChanged.Broadcast(QuestID,ObjectiveID);return true;
 }
 return false;
}
bool UAskirQuestComponent::AdvanceObjective(FName ObjectiveID,int32 Amount)
{
 if(ObjectiveID.IsNone()||Amount<=0)return false;
 // Legacy event API: a matching event advances each active quest once.
 bool Changed=false;
 TArray<FName> IDs;ObjectivesByQuest.GetKeys(IDs);
 for(const FName& QuestID:IDs)Changed=AdvanceQuestObjective(QuestID,ObjectiveID,Amount)||Changed;
 return Changed;
}
bool UAskirQuestComponent::AreQuestObjectivesComplete(FName QuestID)const
{
 if(GetStateByID(QuestID)!=EAskirQuestState::Active)return false;
 const FAskirQuestObjectiveList* List=ObjectivesByQuest.Find(QuestID);
 if(!List||List->Objectives.IsEmpty())return false;
 for(const auto& Objective:List->Objectives)if(Objective.CurrentCount<Objective.RequiredCount)return false;
 return true;
}
bool UAskirQuestComponent::AreObjectivesComplete()const{return AreQuestObjectivesComplete(LegacyActiveQuestID);}
bool UAskirQuestComponent::CompleteQuest(UAskirQuestData* Quest)
{
 if(!IsValid(Quest)||!AreQuestObjectivesComplete(Quest->QuestID))return false;
 QuestStates.Add(Quest->QuestID,EAskirQuestState::Completed);
 ObjectivesByQuest.Remove(Quest->QuestID);
 if(LegacyActiveQuestID==Quest->QuestID){LegacyActiveQuestID=NAME_None;ActiveObjectives.Reset();}
 OnQuestCompleted.Broadcast(Quest->QuestID);return true;
}
EAskirQuestState UAskirQuestComponent::GetStateByID(FName QuestID)const
{
 if(const EAskirQuestState* State=QuestStates.Find(QuestID))return *State;
 return EAskirQuestState::Inactive;
}
EAskirQuestState UAskirQuestComponent::GetState(UAskirQuestData* Quest)const{return IsValid(Quest)?GetStateByID(Quest->QuestID):EAskirQuestState::Inactive;}
TArray<FAskirQuestObjective> UAskirQuestComponent::GetObjectivesForQuest(FName QuestID)const
{
 if(const FAskirQuestObjectiveList* List=ObjectivesByQuest.Find(QuestID))return List->Objectives;
 return {};
}
