#include "Save/AskirSaveGame.h"

bool UAskirSaveGame::IsSafeSlotName(const FString& SlotName)
{
 if(SlotName.IsEmpty() || SlotName.Len()>64)return false;
 for(const TCHAR C:SlotName)
  if(!FChar::IsAlnum(C) && C!=TEXT('_') && C!=TEXT('-'))return false;
 return true;
}

bool UAskirSaveGame::ValidateSnapshot(FString& OutReason) const
{
 OutReason.Reset();
 const auto Fail=[&OutReason](const TCHAR* Reason){OutReason=Reason;return false;};
 if(SaveFormatVersion<1 || SaveFormatVersion>CurrentFormatVersion || SaveGeneration<0)
  return Fail(TEXT("Unsupported version or generation"));
 if(PlayerLocation.ContainsNaN() || PlayerRotation.ContainsNaN() || PlayerLocation.GetAbsMax()>1.e9)
  return Fail(TEXT("Invalid player transform"));
 if(!FMath::IsFinite(PlayerHealth) || PlayerHealth<0.f || !FMath::IsFinite(PlayerStamina) || PlayerStamina<0.f)
  return Fail(TEXT("Invalid player vital values"));
 if(!FMath::IsFinite(SavedWeaponRating) || SavedWeaponRating<0.f || !FMath::IsFinite(SavedArmorRating) || SavedArmorRating<0.f
   || !FMath::IsFinite(SavedGearDurability) || SavedGearDurability<0.f || SavedGearDurability>100.f)
  return Fail(TEXT("Invalid equipment values"));
 if(SavedPlayerGold<0 || SavedWorldDay<1 || SavedWorldHour<0 || SavedWorldHour>23)
  return Fail(TEXT("Invalid economy or world clock"));
 if(SavedProgression.Level<1 || SavedProgression.Level>100 || SavedProgression.Experience<0 || SavedProgression.UnspentPoints<0)
  return Fail(TEXT("Invalid progression"));
 if(SavedProgression.Level<100 && SavedProgression.Experience>=100LL*SavedProgression.Level*SavedProgression.Level)
  return Fail(TEXT("Experience exceeds the current level threshold"));
 const int32 Attributes[]={SavedProgression.Strength,SavedProgression.Agility,SavedProgression.Vitality,SavedProgression.Intellect,SavedProgression.Willpower};
 for(const int32 Value:Attributes)if(Value<1 || Value>100)return Fail(TEXT("Invalid attribute"));
 for(const auto& Pair:SavedProgression.SkillRanks)if(Pair.Key.IsNone() || Pair.Value<0)return Fail(TEXT("Invalid skill rank"));
 for(const auto& Pair:SavedProgression.CraftingKnowledge)if(Pair.Key.IsNone() || Pair.Value<0)return Fail(TEXT("Invalid crafting knowledge"));
 for(const auto& Pair:SavedProgression.Reputation)if(Pair.Key.IsNone() || Pair.Value<-100 || Pair.Value>100)return Fail(TEXT("Invalid progression reputation"));
 if(SavedInventory.Num()>10000)return Fail(TEXT("Inventory exceeds supported entry count"));
 for(const FAskirSavedInventoryEntry& Entry:SavedInventory)
  if(Entry.Quantity<=0 || Entry.ItemAsset.IsNull() || !Entry.ItemAsset.IsValid())return Fail(TEXT("Invalid inventory reference or quantity"));
 const auto ValidObjectives=[](const TArray<FAskirQuestObjective>& Objectives)
 {
  TSet<FName> IDs;
  for(const auto& Objective:Objectives)
  {
   if(Objective.ObjectiveID.IsNone() || IDs.Contains(Objective.ObjectiveID) || Objective.RequiredCount<1
     || Objective.CurrentCount<0 || Objective.CurrentCount>Objective.RequiredCount)return false;
   IDs.Add(Objective.ObjectiveID);
  }
  return true;
 };
 if(!ValidObjectives(SavedActiveObjectives))return Fail(TEXT("Invalid active quest objectives"));
 for(const auto& Pair:SavedQuestStates)
  if(Pair.Key.IsNone() || static_cast<uint8>(Pair.Value)>static_cast<uint8>(EAskirQuestState::Failed))return Fail(TEXT("Invalid quest state"));
 for(const auto& Pair:SavedObjectivesByQuest)
  if(Pair.Key.IsNone() || !ValidObjectives(Pair.Value.Objectives))return Fail(TEXT("Invalid quest objective list"));
 if(static_cast<uint8>(SavedDornfeldMission.Stage)>static_cast<uint8>(EAskirDornfeldStage::Completed))return Fail(TEXT("Invalid Dornfeld stage"));
 if(SaveFormatVersion>=3)
 {
  const auto UniqueIDs=[](const TArray<FName>& IDs)
  {
   TSet<FName> Seen;
   for(FName ID:IDs){if(ID.IsNone() || Seen.Contains(ID))return false;Seen.Add(ID);}
   return true;
  };
  const auto& M=SavedDornfeldMission;
  if(!UniqueIDs(M.ShardIDs) || !UniqueIDs(M.GuardianIDs) || M.ShardIDs.Num()>3 || M.GuardianIDs.Num()>2)return Fail(TEXT("Invalid Dornfeld identity ledger"));
  const int32 Shards=M.ShardIDs.Num(),Guardians=M.GuardianIDs.Num();
  bool bConsistent=false;
  switch(M.Stage)
  {
   case EAskirDornfeldStage::NotStarted:bConsistent=Shards==0 && Guardians==0 && M.ChestID.IsNone() && !M.bRewardClaimed;break;
   case EAskirDornfeldStage::CollectShards:bConsistent=Shards<3 && Guardians==0 && M.ChestID.IsNone() && !M.bRewardClaimed;break;
   case EAskirDornfeldStage::DefeatGuardians:bConsistent=Shards==3 && Guardians<2 && M.ChestID.IsNone() && !M.bRewardClaimed;break;
   case EAskirDornfeldStage::OpenChest:bConsistent=Shards==3 && Guardians==2 && M.ChestID.IsNone() && !M.bRewardClaimed;break;
   case EAskirDornfeldStage::ReturnToElder:bConsistent=Shards==3 && Guardians==2 && !M.ChestID.IsNone() && !M.bRewardClaimed;break;
   case EAskirDornfeldStage::Completed:bConsistent=Shards==3 && Guardians==2 && !M.ChestID.IsNone() && M.bRewardClaimed;break;
   default:break;
  }
  if(!bConsistent)return Fail(TEXT("Dornfeld stage contradicts its receipts"));
  if(M.bRewardClaimed!=SavedClaimedRewards.Contains(FName(TEXT("DORNFELD_BRUNNEN"))))return Fail(TEXT("Dornfeld reward receipt contradicts quest journal"));
  for(FName ID:SavedClaimedRewards)
  {
   const EAskirQuestState* State=SavedQuestStates.Find(ID);
   if(ID.IsNone() || !State || *State!=EAskirQuestState::Completed)return Fail(TEXT("Claimed quest reward without completed quest"));
  }
  for(const auto& Pair:SavedLootByMap)
  {
   if(Pair.Key.IsNone() || !UniqueIDs(Pair.Value.CollectedPickups) || !UniqueIDs(Pair.Value.OpenedChests))return Fail(TEXT("Invalid map loot ledger"));
   for(FName ID:Pair.Value.CollectedPickups)if(Pair.Value.OpenedChests.Contains(ID))return Fail(TEXT("Duplicate map loot identity"));
  }
 }
 if(WorldState.SimulationTick<0 || WorldState.Time.Year<1 || WorldState.Time.Day<1 || WorldState.Time.Day>365
   || WorldState.Time.Hour<0 || WorldState.Time.Hour>23 || WorldState.Time.Minute<0 || WorldState.Time.Minute>59)
  return Fail(TEXT("Invalid simulation time"));
 for(const auto& Pair:WorldState.GlobalValues)if(Pair.Key.IsNone() || !FMath::IsFinite(Pair.Value))return Fail(TEXT("Invalid world value"));
 for(const auto& Pair:SavedFactionReputation)if(Pair.Key.IsNone() || Pair.Value<-100 || Pair.Value>100)return Fail(TEXT("Invalid reputation"));
 for(const auto& Pair:SavedWorldEventLastDays)if(Pair.Key.IsNone() || Pair.Value<-1 || Pair.Value>SavedWorldDay)return Fail(TEXT("Invalid timed event ledger"));
 if(SaveFormatVersion>=3)
 {
  const FAskirPlayerSystemsSave& S=SavedSystems;
  const float Percentages[]={S.Warmth,S.Wetness,S.Exposure,S.Noise,S.Visibility,S.Suspicion,S.Morale,S.Fear,S.Courage,S.TravelFatigue,S.Supplies,S.Shelter,S.Loyalty,S.Bond,S.CompanionStress,S.SurveyProgress};
  for(float Value:Percentages)if(!FMath::IsFinite(Value) || Value<0.f || Value>100.f)return Fail(TEXT("Invalid bounded player system value"));
  const float Amounts[]={S.TravelDistance,S.Fuel,S.Food,S.GuardStamina};
  for(float Value:Amounts)if(!FMath::IsFinite(Value) || Value<0.f)return Fail(TEXT("Invalid player resource"));
  if(S.DiscoveredSites<0 || S.LoreKnowledge<0 || S.Materials<0 || S.CraftedItems<0 || S.CraftSkill<1 || S.MerchantFavor<-100 || S.MerchantFavor>100 || S.TradeVolume<0)
   return Fail(TEXT("Invalid player system counter"));
  const float Needs[]={S.Needs.Hunger,S.Needs.Thirst,S.Needs.Fatigue,S.Needs.Stress};
  for(float Value:Needs)if(!FMath::IsFinite(Value) || Value<0.f || Value>100.f)return Fail(TEXT("Invalid player needs"));
  if(!FMath::IsFinite(S.Needs.Safety) || S.Needs.Safety<0.f || S.Needs.Safety>1.f)return Fail(TEXT("Invalid player safety"));
  for(const auto& Pair:S.FactionStandings)if(Pair.Key.IsNone() || !FMath::IsFinite(Pair.Value) || Pair.Value<-100.f || Pair.Value>100.f)return Fail(TEXT("Invalid faction standing"));
  for(const auto& Pair:S.Relationships)
  {
   const FAskirRelationship& R=Pair.Value;
   const float Values[]={R.Affinity,R.Trust,R.Fear,R.Respect,R.Suspicion};
   if(!Pair.Key.IsValid())return Fail(TEXT("Invalid relationship entity ID"));
   for(float Value:Values)if(!FMath::IsFinite(Value) || Value<-100.f || Value>100.f)return Fail(TEXT("Invalid relationship value"));
  }
 }
 return true;
}
