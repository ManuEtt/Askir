#include "Quest/AskirQuestComponent.h"
#include "Quest/AskirQuestData.h"
#include "Trade/AskirTradeComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Quest/AskirQuestRewardRules.h"
UAskirQuestComponent::UAskirQuestComponent(){PrimaryComponentTick.bCanEverTick=false;}
bool UAskirQuestComponent::StartQuest(UAskirQuestData* Quest)
{
 if(bRewardTransaction||!IsValid(Quest)||Quest->QuestID.IsNone()||Quest->Objectives.IsEmpty()||HasClaimedReward(Quest->QuestID))return false;
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
 if(bRewardTransaction||QuestID.IsNone()||ObjectiveID.IsNone()||Amount<=0||GetStateByID(QuestID)!=EAskirQuestState::Active)return false;
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
 TSet<FName> Seen;
 for(const auto& Objective:List->Objectives)
 {
  if(Objective.ObjectiveID.IsNone() || Seen.Contains(Objective.ObjectiveID) || Objective.RequiredCount<=0
   || Objective.CurrentCount!=Objective.RequiredCount)return false;
  Seen.Add(Objective.ObjectiveID);
 }
 return true;
}
bool UAskirQuestComponent::AreObjectivesComplete()const{return AreQuestObjectivesComplete(LegacyActiveQuestID);}
bool UAskirQuestComponent::CompleteQuest(UAskirQuestData* Quest)
{
 if(bRewardTransaction||!IsValid(Quest)||!AreQuestObjectivesComplete(Quest->QuestID))return false;
 QuestStates.Add(Quest->QuestID,EAskirQuestState::Completed);
 ObjectivesByQuest.Remove(Quest->QuestID);
 RefreshLegacyObjectives();
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

bool UAskirQuestComponent::HasClaimedReward(FName QuestID)const
{
 return !QuestID.IsNone()&&ClaimedRewards.Contains(QuestID);
}
bool UAskirQuestComponent::TurnInQuestWithRewards(UAskirQuestData* Quest)
{
 if(bRewardTransaction||!IsValid(Quest)||Quest->QuestID.IsNone()||HasClaimedReward(Quest->QuestID)||!AreQuestObjectivesComplete(Quest->QuestID))return false;
 const FName QuestID=Quest->QuestID;
 const int32 GoldReward=Quest->GoldReward,ExperienceReward=Quest->ExperienceReward,RewardCount=Quest->RewardItemQuantity;
 UAskirItemData* const RewardItem=Quest->RewardItem.Get();
 AActor* Owner=GetOwner();if(!IsValid(Owner))return false;
 UAskirTradeComponent* Wallet=Owner->FindComponentByClass<UAskirTradeComponent>();
 UAskirProgressionComponent* Progression=Owner->FindComponentByClass<UAskirProgressionComponent>();
 UAskirInventoryComponent* Bag=Owner->FindComponentByClass<UAskirInventoryComponent>();
 if(GoldReward<0||ExperienceReward<0||RewardCount<0)return false;
 if(GoldReward>0&&(!Wallet||Wallet->Gold<0||int64(Wallet->Gold)+GoldReward>MAX_int32))return false;
 if(ExperienceReward>0&&!Progression)return false;
 if(RewardCount>0&&(!IsValid(RewardItem)||!Bag))return false;
 AskirQuestRewardRules::Progress PreparedProgress;
 if(ExperienceReward>0 && !AskirQuestRewardRules::PrepareExperience(
  {Progression->Progress.Level,Progression->Progress.Experience,Progression->Progress.UnspentPoints},
  ExperienceReward,PreparedProgress))return false;
 TArray<FAskirInventoryEntry> PreparedInventory;
 if(RewardCount>0)
 {
  FAskirInventoryEntry Reward;Reward.Item=RewardItem;Reward.Quantity=RewardCount;
  if(!Bag->PrepareExchange({}, {Reward}, PreparedInventory))return false;
 }
 // Inventory notifications may re-enter a turn-in. Keep the transaction closed until
 // all rewards and the receipt have been committed, then broadcast quest completion.
 TGuardValue<bool> TransactionGuard(bRewardTransaction,true);
 bool UnusedInventoryGuard=false;
 TGuardValue<bool> InventoryGuard(RewardCount>0?Bag->bTransactionInProgress:UnusedInventoryGuard,true);
 if(RewardCount>0)Bag->Items=MoveTemp(PreparedInventory);
 QuestStates.Add(QuestID,EAskirQuestState::Completed);
 ObjectivesByQuest.Remove(QuestID);
 ClaimedRewards.Add(QuestID);
 RefreshLegacyObjectives();
 if(GoldReward>0)Wallet->Gold+=GoldReward;
 if(ExperienceReward>0)
 {
  Progression->Progress.Level=PreparedProgress.Level;
  Progression->Progress.Experience=PreparedProgress.Experience;
  Progression->Progress.UnspentPoints=PreparedProgress.UnspentPoints;
 }
 // Commit every reward and the receipt before the first external callback. The
 // quest transaction guard also blocks snapshots throughout all notifications.
 if(ExperienceReward>0)Progression->OnProgressionChanged.Broadcast();
 if(RewardCount>0)Bag->OnInventoryChanged.Broadcast();
 OnQuestCompleted.Broadcast(QuestID);
 return true;
}

void UAskirQuestComponent::RefreshLegacyObjectives()
{
 if(GetStateByID(LegacyActiveQuestID)!=EAskirQuestState::Active||!ObjectivesByQuest.Contains(LegacyActiveQuestID))
 {
  LegacyActiveQuestID=NAME_None;
  TArray<FName> IDs;
  ObjectivesByQuest.GetKeys(IDs);
  IDs.Sort(FNameLexicalLess());
  for(FName ID:IDs)if(GetStateByID(ID)==EAskirQuestState::Active){LegacyActiveQuestID=ID;break;}
 }
 ActiveObjectives=GetObjectivesForQuest(LegacyActiveQuestID);
}
bool UAskirQuestComponent::RestoreQuestProgress(FName QuestID,const TArray<FAskirQuestObjective>& Objectives,EAskirQuestState State,bool bRewardClaimed,bool bNotify)
{
 if(bRewardTransaction||QuestID.IsNone()||uint8(State)>uint8(EAskirQuestState::Failed))return false;
 if(State==EAskirQuestState::Active&&Objectives.IsEmpty())return false;
 TSet<FName> Seen;
 for(const FAskirQuestObjective& Objective:Objectives)
 {
  if(Objective.ObjectiveID.IsNone()||Seen.Contains(Objective.ObjectiveID)||Objective.RequiredCount<=0
   ||Objective.CurrentCount<0||Objective.CurrentCount>Objective.RequiredCount)return false;
  Seen.Add(Objective.ObjectiveID);
 }
 const EAskirQuestState Previous=GetStateByID(QuestID);
 if(bRewardClaimed)State=EAskirQuestState::Completed;
 if(State==EAskirQuestState::Inactive)QuestStates.Remove(QuestID);
 else QuestStates.Add(QuestID,State);
 if(State==EAskirQuestState::Active)
 {
  FAskirQuestObjectiveList List;List.Objectives=Objectives;
  ObjectivesByQuest.Add(QuestID,List);
 }
 else ObjectivesByQuest.Remove(QuestID);
 if(bRewardClaimed)ClaimedRewards.Add(QuestID);
 else ClaimedRewards.Remove(QuestID);
 RefreshLegacyObjectives();
 if(bNotify)
 {
  OnQuestProgressChanged.Broadcast(QuestID,NAME_None);
  if(State==EAskirQuestState::Completed&&Previous!=State)OnQuestCompleted.Broadcast(QuestID);
 }
 return true;
}
void UAskirQuestComponent::NormalizeRestoredState(bool bNotify)
{
 if(bRewardTransaction)return;
 ClaimedRewards.Remove(NAME_None);
 ConsumedQuestTriggers.Remove(NAME_None);
 CompletedEncounters.Remove(NAME_None);
 QuestStates.Remove(NAME_None);
 ObjectivesByQuest.Remove(NAME_None);
 // V60-era saves retained only the legacy objective mirror.
 if(!LegacyActiveQuestID.IsNone()&&GetStateByID(LegacyActiveQuestID)==EAskirQuestState::Active
  &&!ObjectivesByQuest.Contains(LegacyActiveQuestID)&&!ActiveObjectives.IsEmpty())
 {
  FAskirQuestObjectiveList List;List.Objectives=ActiveObjectives;
  ObjectivesByQuest.Add(LegacyActiveQuestID,List);
 }
 for(auto It=QuestStates.CreateIterator();It;++It)
  if(uint8(It.Value())>uint8(EAskirQuestState::Failed))It.RemoveCurrent();
 for(FName ID:ClaimedRewards)QuestStates.Add(ID,EAskirQuestState::Completed);
 for(auto It=ObjectivesByQuest.CreateIterator();It;++It)
 {
  if(GetStateByID(It.Key())!=EAskirQuestState::Active){It.RemoveCurrent();continue;}
  TSet<FName> Seen;
  bool bInvalid=It.Value().Objectives.IsEmpty();
  for(auto& Objective:It.Value().Objectives)
  {
   if(Objective.ObjectiveID.IsNone()||Objective.RequiredCount<=0||Seen.Contains(Objective.ObjectiveID))bInvalid=true;
   Seen.Add(Objective.ObjectiveID);
   Objective.CurrentCount=FMath::Clamp(Objective.CurrentCount,0,FMath::Max(0,Objective.RequiredCount));
  }
  if(bInvalid){QuestStates.Add(It.Key(),EAskirQuestState::Failed);It.RemoveCurrent();}
 }
 for(auto& Pair:QuestStates)
  if(Pair.Value==EAskirQuestState::Active&&!ObjectivesByQuest.Contains(Pair.Key))Pair.Value=EAskirQuestState::Failed;
 RefreshLegacyObjectives();
 if(bNotify)OnQuestProgressChanged.Broadcast(NAME_None,NAME_None);
}
