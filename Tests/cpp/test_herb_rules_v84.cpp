#include "Source/Askir/Mission/AskirHerbRules.h"
#include <cstdint>
#include <iostream>
#include <limits>

namespace R=AskirHerbRules;
int Checks=0,Failures=0;
void Check(bool Passed,const char* Message)
{++Checks;if(!Passed){++Failures;std::cerr<<"FAIL: "<<Message<<'\n';}}
int main()
{
    using R::Stage;
    Check(R::GoldReward==24&&R::ExperienceReward==150&&R::ItemRewardQuantity==1,"Canonical SQ_003 reward");
    for(unsigned Mask=0;Mask<16;++Mask)
    {
        const bool Active=Mask&1,Completed=Mask&2,Claimed=Mask&4,Explored=Mask&8;
        const Stage Expected=Completed||Claimed?Stage::Completed:!Active?Stage::NotStarted:Explored?Stage::ReturnToRurik:Stage::ExploreMeadow;
        Check(R::ResolveStage(Active,Completed,Claimed,Explored)==Expected,"Durable ledger determines stage");
    }
    for(unsigned Current=0;Current<4;++Current)
    {
        const auto StageValue=static_cast<Stage>(Current);
        for(unsigned Mask=0;Mask<8;++Mask)
        {
            const bool Valid=Mask&1,Duplicate=Mask&2,Mutable=Mask&4;
            Check(R::CanAccept(StageValue,Valid,Duplicate,Mutable)==(StageValue==Stage::NotStarted&&Valid&&!Duplicate&&Mutable),"Only first valid unclaimed acceptance mutates");
            Check(R::CanExplore(StageValue,Valid,Duplicate,Mutable)==(StageValue==Stage::ExploreMeadow&&Valid&&!Duplicate&&Mutable),"Only accepted canonical unique exploration mutates");
        }
        for(unsigned Mask=0;Mask<64;++Mask)
        {
            const bool Claimed=Mask&1,Mutable=Mask&2,Asset=Mask&4,Bag=Mask&8,Wallet=Mask&16,XP=Mask&32;
            for(const std::int64_t Gold:{std::int64_t(-1),std::int64_t(0),std::int64_t(std::numeric_limits<std::int32_t>::max())-24,
                std::int64_t(std::numeric_limits<std::int32_t>::max())-23,std::numeric_limits<std::int64_t>::max()})
            {
                const bool Expected=StageValue==Stage::ReturnToRurik&&!Claimed&&Mutable&&Asset&&Bag&&Wallet&&XP&&
                    Gold>=0&&Gold<=std::int64_t(std::numeric_limits<std::int32_t>::max())-24;
                Check(R::CanTurnIn(StageValue,Claimed,Mutable,Asset,Bag,Wallet,XP,Gold)==Expected,"Whole payout rejects missing systems, duplication and wallet overflow");
            }
        }
    }
    for(unsigned Mask=0;Mask<64;++Mask)
    {
        const bool Accepted=Mask&1,Help=Mask&2,Bargain=Mask&4,Active=Mask&8,Completed=Mask&16,Claimed=Mask&32;
        const bool Empty=!Accepted&&!Help&&!Bargain&&!Active&&!Completed&&!Claimed;
        const bool InProgress=Accepted&&(Help!=Bargain)&&Active&&!Completed&&!Claimed;
        const bool Finished=Accepted&&(Help!=Bargain)&&!Active&&Completed&&Claimed;
        Check(R::ValidateReceipt(Accepted,Help,Bargain,Active,Completed,Claimed)==(Empty||InProgress||Finished),"Accepted choice and reward receipts are consistent");
    }
    for(unsigned Mask=0;Mask<8;++Mask)for(int SourceCount=-1;SourceCount<=2;++SourceCount)
    {
        const bool Accepted=Mask&1,Explored=Mask&2,Completed=Mask&4;
        const bool Empty=!Accepted&&!Explored&&!Completed&&SourceCount==0;
        const bool Exploring=Accepted&&!Explored&&!Completed&&SourceCount==0;
        const bool ReturningOrComplete=Accepted&&Explored&&SourceCount==1;
        Check(R::ValidateExplorationReceipts(Accepted,Explored,SourceCount,Completed)==(Empty||Exploring||ReturningOrComplete),"One canonical source proves exploration, never anonymous or duplicate sources");
    }
    for(int ExploreRequired=0;ExploreRequired<=2;++ExploreRequired)for(int ExploreCurrent=-1;ExploreCurrent<=2;++ExploreCurrent)
        for(int ReturnRequired=0;ReturnRequired<=2;++ReturnRequired)for(int ReturnCurrent=-1;ReturnCurrent<=2;++ReturnCurrent)
            for(bool Explored:{false,true})
                Check(R::ValidateActiveObjectiveMirror(ExploreRequired,ExploreCurrent,ReturnRequired,ReturnCurrent,Explored)==
                    (ExploreRequired==1&&ExploreCurrent==(Explored?1:0)&&ReturnRequired==1&&ReturnCurrent==0),"Failed payout cannot retain completed return objective");
    std::cout<<"Herb V84: "<<Checks<<" assertions, "<<Failures<<" failures. Unreal runtime NOT_EXECUTED.\n";
    return Failures?1:0;
}
