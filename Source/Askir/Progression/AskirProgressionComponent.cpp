#include "Progression/AskirProgressionComponent.h"
UAskirProgressionComponent::UAskirProgressionComponent(){PrimaryComponentTick.bCanEverTick=false;}
int32 UAskirProgressionComponent::LevelThreshold(int32 Level)
{
 const int64 L=FMath::Max(1,Level);
 return static_cast<int32>(FMath::Min<int64>(INT32_MAX,100LL*L*L));
}
int32 UAskirProgressionComponent::ExperienceToNextLevel() const
{return FMath::Max(0,LevelThreshold(Progress.Level)-Progress.Experience);}
void UAskirProgressionComponent::AwardExperience(int32 Amount)
{
 if(Amount<=0)return;
 Progress.Experience=static_cast<int32>(FMath::Min<int64>(INT32_MAX,static_cast<int64>(Progress.Experience)+Amount));
 while(Progress.Level<100 && Progress.Experience>=LevelThreshold(Progress.Level))
 {Progress.Experience-=LevelThreshold(Progress.Level);++Progress.Level;++Progress.UnspentPoints;}
 OnProgressionChanged.Broadcast();
}
bool UAskirProgressionComponent::SpendAttributePoint(EAskirAttribute Attribute)
{
 if(Progress.UnspentPoints<=0)return false;
 int32* Value=nullptr;
 switch(Attribute){
 case EAskirAttribute::Strength:Value=&Progress.Strength;break;
 case EAskirAttribute::Agility:Value=&Progress.Agility;break;
 case EAskirAttribute::Vitality:Value=&Progress.Vitality;break;
 case EAskirAttribute::Intellect:Value=&Progress.Intellect;break;
 case EAskirAttribute::Willpower:Value=&Progress.Willpower;break;
 default:return false;}
 if(*Value>=100)return false;
 ++(*Value);--Progress.UnspentPoints;OnProgressionChanged.Broadcast();return true;
}
bool UAskirProgressionComponent::TrainSkill(FName Skill,int32 MaximumRank)
{
 if(Skill.IsNone()||Progress.UnspentPoints<=0||MaximumRank<=0)return false;
 int32& Rank=Progress.SkillRanks.FindOrAdd(Skill);
 if(Rank>=MaximumRank)return false;
 ++Rank;--Progress.UnspentPoints;OnProgressionChanged.Broadcast();return true;
}
bool UAskirProgressionComponent::UnlockPerk(FName Perk,FName RequiredPerk,int32 MinimumLevel)
{
 if(Perk.IsNone()||Progress.UnspentPoints<=0||Progress.Level<FMath::Max(1,MinimumLevel)||Progress.UnlockedPerks.Contains(Perk))return false;
 if(!RequiredPerk.IsNone()&&!Progress.UnlockedPerks.Contains(RequiredPerk))return false;
 Progress.UnlockedPerks.Add(Perk);--Progress.UnspentPoints;OnProgressionChanged.Broadcast();return true;
}
void UAskirProgressionComponent::LearnRecipe(FName Recipe)
{
 if(Recipe.IsNone())return;
 int32& Rank=Progress.CraftingKnowledge.FindOrAdd(Recipe);
 if(Rank==0){Rank=1;OnProgressionChanged.Broadcast();}
}
bool UAskirProgressionComponent::KnowsRecipe(FName Recipe) const
{return Progress.CraftingKnowledge.Contains(Recipe);}
void UAskirProgressionComponent::ChangeReputation(FName Faction,int32 Delta)
{
 if(Faction.IsNone()||Delta==0)return;
 int32& Standing=Progress.Reputation.FindOrAdd(Faction);
 Standing=static_cast<int32>(FMath::Clamp<int64>(static_cast<int64>(Standing)+Delta,-100,100));
 OnProgressionChanged.Broadcast();
}
int32 UAskirProgressionComponent::GetReputation(FName Faction) const
{const int32* Value=Progress.Reputation.Find(Faction);return Value?*Value:0;}
float UAskirProgressionComponent::GetAttributeBonus(EAskirAttribute Attribute) const
{
 int32 Value=5;
 switch(Attribute){
 case EAskirAttribute::Strength:Value=Progress.Strength;break;
 case EAskirAttribute::Agility:Value=Progress.Agility;break;
 case EAskirAttribute::Vitality:Value=Progress.Vitality;break;
 case EAskirAttribute::Intellect:Value=Progress.Intellect;break;
 case EAskirAttribute::Willpower:Value=Progress.Willpower;break;
 default:break;}
 return (Value-5)*0.05f;
}
float UAskirProgressionComponent::GetSkillSuccessChance(FName Skill,int32 Difficulty) const
{
 const int32* Rank=Progress.SkillRanks.Find(Skill);
 const float Base=0.5f+(Rank?*Rank:0)*0.05f-Difficulty*0.025f;
 return FMath::Clamp(Base,0.05f,0.95f);
}
void UAskirProgressionComponent::RestoreProgress(const FAskirProgressionSnapshot& Snapshot)
{
 Progress=Snapshot;
 Progress.Level=FMath::Clamp(Progress.Level,1,100);
 Progress.Experience=FMath::Clamp(Progress.Experience,0,LevelThreshold(Progress.Level)-1);
 Progress.UnspentPoints=FMath::Max(0,Progress.UnspentPoints);
 Progress.Strength=FMath::Clamp(Progress.Strength,1,100);
 Progress.Agility=FMath::Clamp(Progress.Agility,1,100);
 Progress.Vitality=FMath::Clamp(Progress.Vitality,1,100);
 Progress.Intellect=FMath::Clamp(Progress.Intellect,1,100);
 Progress.Willpower=FMath::Clamp(Progress.Willpower,1,100);
 OnProgressionChanged.Broadcast();
}
