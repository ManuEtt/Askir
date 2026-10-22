#pragma once
#include <cstdint>
#include <limits>

/** SQ_003 production decisions shared by native gameplay and portable tests. */
namespace AskirHerbRules
{
enum class Stage : std::uint8_t { NotStarted, ExploreMeadow, ReturnToRurik, Completed };
constexpr int GoldReward=24,ExperienceReward=150,ItemRewardQuantity=1;
constexpr Stage ResolveStage(bool Active,bool Completed,bool Claimed,bool Explored)
{
    if(Completed||Claimed)return Stage::Completed;
    if(!Active)return Stage::NotStarted;
    return Explored?Stage::ReturnToRurik:Stage::ExploreMeadow;
}
constexpr bool CanAccept(Stage Current,bool ValidChoice,bool AcceptedReceipt,bool CanMutate)
{return Current==Stage::NotStarted&&ValidChoice&&!AcceptedReceipt&&CanMutate;}
constexpr bool CanExplore(Stage Current,bool CanonicalSource,bool Duplicate,bool CanMutate)
{return Current==Stage::ExploreMeadow&&CanonicalSource&&!Duplicate&&CanMutate;}
constexpr bool CanTurnIn(Stage Current,bool Claimed,bool CanMutate,bool RewardAsset,bool Bag,bool Wallet,bool Progression,std::int64_t Gold)
{
    return Current==Stage::ReturnToRurik&&!Claimed&&CanMutate&&RewardAsset&&Bag&&Wallet&&Progression&&
        Gold>=0&&Gold<=std::numeric_limits<std::int32_t>::max()-GoldReward;
}
constexpr bool ValidateReceipt(bool Accepted,bool Help,bool Bargain,bool Active,bool Completed,bool Claimed)
{
    if(Help&&Bargain)return false;
    if(Completed!=Claimed||(Active&&Completed))return false;
    if(Accepted)return (Help!=Bargain)&&(Active||Completed);
    return !Help&&!Bargain&&!Active&&!Completed&&!Claimed;
}
constexpr bool ValidateExplorationReceipts(bool Accepted,bool Explored,int CanonicalSourceCount,bool Completed)
{
    if(CanonicalSourceCount<0||CanonicalSourceCount>1)return false;
    if(!Accepted&&(Explored||CanonicalSourceCount!=0))return false;
    if(Explored!=(CanonicalSourceCount==1))return false;
    return !Completed||(Accepted&&Explored);
}
constexpr bool ValidateActiveObjectiveMirror(int ExploreRequired,int ExploreCurrent,int ReturnRequired,int ReturnCurrent,bool Explored)
{
    return ExploreRequired==1&&ExploreCurrent==(Explored?1:0)&&ReturnRequired==1&&ReturnCurrent==0;
}
}
