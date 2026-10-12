#include "Mission/AskirDornfeldMissionComponent.h"
#include "Mission/AskirDornfeldRules.h"
#include "Trade/AskirTradeComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "GameFramework/Actor.h"

namespace
{
using RulesStage = AskirDornfeldRules::Stage;
static_assert(static_cast<uint8>(EAskirDornfeldStage::Completed) == static_cast<uint8>(RulesStage::Completed), "Mission stage mapping changed");
RulesStage ToRules(EAskirDornfeldStage Stage) { return static_cast<RulesStage>(Stage); }
void NormalizeIDs(TArray<FName>& IDs, int32 Maximum)
{
 TArray<FName> Unique;
 for (FName ID : IDs)
 {
  if (!ID.IsNone()) Unique.AddUnique(ID);
  if (Unique.Num() == Maximum) break;
 }
 IDs = MoveTemp(Unique);
}
}

UAskirDornfeldMissionComponent::UAskirDornfeldMissionComponent(){PrimaryComponentTick.bCanEverTick=false;}
void UAskirDornfeldMissionComponent::SyncQuestJournal(bool bNotify)
{
 if (!GetOwner()) return;
 UAskirQuestComponent* Journal = GetOwner()->FindComponentByClass<UAskirQuestComponent>();
 if (!Journal) return;
 const FName QuestID(TEXT("DORNFELD_BRUNNEN"));
 if (State.Stage == EAskirDornfeldStage::NotStarted)
 {
  Journal->RestoreQuestProgress(QuestID, {}, EAskirQuestState::Inactive, false,bNotify);
  return;
 }
 TArray<FAskirQuestObjective> Objectives;
 const auto AddObjective = [&Objectives](const TCHAR* ID, FText Text, int32 Required, int32 Current)
 {
  FAskirQuestObjective Objective;
  Objective.ObjectiveID = FName(ID); Objective.Description = Text;
  Objective.RequiredCount = Required; Objective.CurrentCount = Current;
  Objectives.Add(Objective);
 };
 AddObjective(TEXT("TALK_ELDER"), NSLOCTEXT("AskirDornfeld", "Accept", "Sprich mit Mara."), 1, 1);
 AddObjective(TEXT("COLLECT_SHARDS"), NSLOCTEXT("AskirDornfeld", "Collect", "Sammle drei unterschiedliche Brunnenscherben."), 3, State.ShardIDs.Num());
 AddObjective(TEXT("DEFEAT_GUARDIANS"), NSLOCTEXT("AskirDornfeld", "Defeat", "Besiege die beiden Schattenwächter."), 2, State.GuardianIDs.Num());
 AddObjective(TEXT("OPEN_CHEST"), NSLOCTEXT("AskirDornfeld", "Open", "Öffne die Kellertruhe."), 1, State.ChestID.IsNone() ? 0 : 1);
 AddObjective(TEXT("RETURN_TO_ELDER"), NSLOCTEXT("AskirDornfeld", "TurnIn", "Kehre zu Mara zurück."), 1, State.bRewardClaimed ? 1 : 0);
 Journal->RestoreQuestProgress(QuestID, Objectives, State.bRewardClaimed ? EAskirQuestState::Completed : EAskirQuestState::Active, State.bRewardClaimed,bNotify);
}
void UAskirDornfeldMissionComponent::BroadcastObjective()
{
 SyncQuestJournal();
 OnObjectiveChanged.Broadcast(GetObjectiveText(),State.Stage);
}
void UAskirDornfeldMissionComponent::SetStage(EAskirDornfeldStage Next)
{
 if(State.Stage==Next)return;
 State.Stage=Next;
 SyncQuestJournal();
 OnStageChanged.Broadcast(Next);
 OnObjectiveChanged.Broadcast(GetObjectiveText(),State.Stage);
}
bool UAskirDornfeldMissionComponent::TalkToElder()
{
 if(bClaimingReward)return false;
 if(State.Stage==EAskirDornfeldStage::NotStarted){SetStage(EAskirDornfeldStage::CollectShards);return true;}
 AActor* Owner=GetOwner();if(!IsValid(Owner))return false;
 UAskirTradeComponent* Wallet=Owner->FindComponentByClass<UAskirTradeComponent>();
 UAskirProgressionComponent* XP=Owner->FindComponentByClass<UAskirProgressionComponent>();
 if(!AskirDornfeldRules::CanClaimReward(ToRules(State.Stage),State.bRewardClaimed,IsValid(Wallet),IsValid(XP),Wallet?Wallet->Gold:0))return false;
 TGuardValue<bool> ClaimGuard(bClaimingReward,true);
 // Commit the receipt before any XP/quest/UI delegate can re-enter the turn-in.
 State.bRewardClaimed=true;
 State.Stage=EAskirDornfeldStage::Completed;
 // The SaveGame snapshot must already contain both reward receipts if an XP
 // listener saves immediately. Publish journal/UI delegates after payout.
 SyncQuestJournal(false);
 Wallet->EarnGold(AskirDornfeldRules::GoldReward);
 XP->AwardExperience(AskirDornfeldRules::ExperienceReward);
 SyncQuestJournal();
 if(UAskirQuestComponent* Journal=Owner->FindComponentByClass<UAskirQuestComponent>())
  Journal->OnQuestCompleted.Broadcast(TEXT("DORNFELD_BRUNNEN"));
 OnStageChanged.Broadcast(State.Stage);
 OnObjectiveChanged.Broadcast(GetObjectiveText(),State.Stage);
 return true;
}
bool UAskirDornfeldMissionComponent::CanCollectShard(FName ID) const
{
 return AskirDornfeldRules::CanCollectShard(ToRules(State.Stage),!ID.IsNone(),State.ShardIDs.Contains(ID),State.ShardIDs.Num());
}
bool UAskirDornfeldMissionComponent::CanDefeatGuardian(FName ID) const
{
 return AskirDornfeldRules::CanDefeatGuardian(ToRules(State.Stage),!ID.IsNone(),State.GuardianIDs.Contains(ID),State.GuardianIDs.Num());
}
bool UAskirDornfeldMissionComponent::CanOpenMissionChest(FName ID) const
{
 return AskirDornfeldRules::CanOpenChest(ToRules(State.Stage),!ID.IsNone());
}
bool UAskirDornfeldMissionComponent::CollectShard(FName ID,bool bNotify)
{
 if(!CanCollectShard(ID))return false;
 State.ShardIDs.Add(ID);
 if(!bNotify)
 {
  if(State.ShardIDs.Num()==AskirDornfeldRules::RequiredShards)State.Stage=EAskirDornfeldStage::DefeatGuardians;
  SyncQuestJournal(false);
  return true;
 }
 if(State.ShardIDs.Num()==AskirDornfeldRules::RequiredShards)SetStage(EAskirDornfeldStage::DefeatGuardians);
 else BroadcastObjective();
 return true;
}
bool UAskirDornfeldMissionComponent::DefeatGuardian(FName ID)
{
 if(!CanDefeatGuardian(ID))return false;
 State.GuardianIDs.Add(ID);
 if(State.GuardianIDs.Num()==AskirDornfeldRules::RequiredGuardians)SetStage(EAskirDornfeldStage::OpenChest);
 else BroadcastObjective();
 return true;
}
bool UAskirDornfeldMissionComponent::OpenMissionChest(FName ID,bool bNotify)
{
 if(!CanOpenMissionChest(ID))return false;
 State.ChestID=ID;
 if(!bNotify){State.Stage=EAskirDornfeldStage::ReturnToElder;SyncQuestJournal(false);return true;}
 SetStage(EAskirDornfeldStage::ReturnToElder);
 return true;
}
FText UAskirDornfeldMissionComponent::GetObjectiveText()const
{
 switch(State.Stage)
 {
 case EAskirDornfeldStage::NotStarted:return NSLOCTEXT("AskirDornfeld","TalkStart","Sprich mit Dorfältester Mara.");
 case EAskirDornfeldStage::CollectShards:return FText::Format(NSLOCTEXT("AskirDornfeld","Shards","Sammle Brunnenscherben ({0}/3)."),FText::AsNumber(State.ShardIDs.Num()));
 case EAskirDornfeldStage::DefeatGuardians:return FText::Format(NSLOCTEXT("AskirDornfeld","Guardians","Besiege Schattenwächter ({0}/2)."),FText::AsNumber(State.GuardianIDs.Num()));
 case EAskirDornfeldStage::OpenChest:return NSLOCTEXT("AskirDornfeld","Chest","Öffne die Truhe im verlassenen Keller.");
 case EAskirDornfeldStage::ReturnToElder:return NSLOCTEXT("AskirDornfeld","Return","Kehre zu Mara zurück.");
 default:return NSLOCTEXT("AskirDornfeld","Done","Dornfeld ist vorerst sicher.");
 }
}
void UAskirDornfeldMissionComponent::RestoreMission(const FAskirDornfeldMissionSave& Saved,bool bNotify)
{
 if(bClaimingReward)return;
 State=Saved;
 NormalizeIDs(State.ShardIDs,AskirDornfeldRules::RequiredShards);
 NormalizeIDs(State.GuardianIDs,AskirDornfeldRules::RequiredGuardians);
 State.Stage=static_cast<EAskirDornfeldStage>(AskirDornfeldRules::NormalizeStage(static_cast<uint8>(Saved.Stage),State.ShardIDs.Num(),State.GuardianIDs.Num(),!State.ChestID.IsNone(),State.bRewardClaimed));
 if(!State.bRewardClaimed)
 {
  if(State.Stage==EAskirDornfeldStage::NotStarted)State.ShardIDs.Reset();
  if(State.ShardIDs.Num()<AskirDornfeldRules::RequiredShards)State.GuardianIDs.Reset();
  if(State.GuardianIDs.Num()<AskirDornfeldRules::RequiredGuardians)State.ChestID=NAME_None;
 }
 SyncQuestJournal(bNotify);
 if(bNotify)
 {
  OnStageChanged.Broadcast(State.Stage);
  OnObjectiveChanged.Broadcast(GetObjectiveText(),State.Stage);
 }
}
void UAskirDornfeldMissionComponent::NotifyMissionRestored()
{
 SyncQuestJournal();
 OnStageChanged.Broadcast(State.Stage);
 OnObjectiveChanged.Broadcast(GetObjectiveText(),State.Stage);
}
