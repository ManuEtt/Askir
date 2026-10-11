#include "Mission/AskirDornfeldMissionComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "Progression/AskirProgressionComponent.h"

UAskirDornfeldMissionComponent::UAskirDornfeldMissionComponent(){PrimaryComponentTick.bCanEverTick=false;}
void UAskirDornfeldMissionComponent::BroadcastObjective(){OnObjectiveChanged.Broadcast(GetObjectiveText(),State.Stage);}
void UAskirDornfeldMissionComponent::SetStage(EAskirDornfeldStage Next)
{
 if(State.Stage==Next)return;
 State.Stage=Next;
 OnStageChanged.Broadcast(Next);
 BroadcastObjective();
}
bool UAskirDornfeldMissionComponent::TalkToElder()
{
 if(State.Stage==EAskirDornfeldStage::NotStarted){SetStage(EAskirDornfeldStage::CollectShards);return true;}
 if(State.Stage!=EAskirDornfeldStage::ReturnToElder||State.bRewardClaimed)return false;
 AActor* Owner=GetOwner();if(!IsValid(Owner))return false;
 UAskirTradeComponent* Wallet=Owner->FindComponentByClass<UAskirTradeComponent>();
 UAskirProgressionComponent* XP=Owner->FindComponentByClass<UAskirProgressionComponent>();
 if(!Wallet||!XP||Wallet->Gold>MAX_int32-90)return false;
 // Mark completion before awarding; duplicate interactions cannot grant a second reward.
 State.bRewardClaimed=true;
 SetStage(EAskirDornfeldStage::Completed);
 Wallet->EarnGold(90);
 XP->AwardExperience(180);
 return true;
}
bool UAskirDornfeldMissionComponent::CollectShard(FName ID)
{
 if(State.Stage!=EAskirDornfeldStage::CollectShards||ID.IsNone()||State.ShardIDs.Contains(ID))return false;
 State.ShardIDs.Add(ID);
 if(State.ShardIDs.Num()>=3)SetStage(EAskirDornfeldStage::DefeatGuardians);
 else BroadcastObjective();
 return true;
}
bool UAskirDornfeldMissionComponent::DefeatGuardian(FName ID)
{
 if(State.Stage!=EAskirDornfeldStage::DefeatGuardians||ID.IsNone()||State.GuardianIDs.Contains(ID))return false;
 State.GuardianIDs.Add(ID);
 if(State.GuardianIDs.Num()>=2)SetStage(EAskirDornfeldStage::OpenChest);
 else BroadcastObjective();
 return true;
}
bool UAskirDornfeldMissionComponent::OpenMissionChest(FName ID)
{
 if(State.Stage!=EAskirDornfeldStage::OpenChest||ID.IsNone())return false;
 State.ChestID=ID;
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
void UAskirDornfeldMissionComponent::RestoreMission(const FAskirDornfeldMissionSave& Saved)
{
 // Saved data is untrusted: normalize IDs and stage so invalid saves cannot multiply rewards.
 State=Saved;
 State.ShardIDs.Remove(NAME_None);
 State.GuardianIDs.Remove(NAME_None);
 TSet<FName> UniqueShards;for(FName ID:State.ShardIDs)UniqueShards.Add(ID);
 TSet<FName> UniqueGuardians;for(FName ID:State.GuardianIDs)UniqueGuardians.Add(ID);
 State.ShardIDs=UniqueShards.Array();State.GuardianIDs=UniqueGuardians.Array();
 if(State.ShardIDs.Num()>3)State.ShardIDs.SetNum(3);
 if(State.GuardianIDs.Num()>2)State.GuardianIDs.SetNum(2);
 if(State.bRewardClaimed)State.Stage=EAskirDornfeldStage::Completed;
 else if(State.Stage==EAskirDornfeldStage::Completed)State.Stage=EAskirDornfeldStage::ReturnToElder;
 OnStageChanged.Broadcast(State.Stage);
 BroadcastObjective();
}
