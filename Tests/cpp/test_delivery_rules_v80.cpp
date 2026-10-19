#include "Source/Askir/Mission/AskirDeliveryRules.h"
#include <cstdint>
#include <iostream>
#include <limits>

namespace R=AskirDeliveryRules;
int Checks=0,Failures=0;
void Check(bool Result,const char* Description)
{
    ++Checks;if(!Result){++Failures;std::cerr<<"FAIL: "<<Description<<'\n';}
}
int main()
{
    using R::Stage;
    Check(R::RequiredBandits==2 && R::GoldReward==16 && R::ExperienceReward==100 && R::ItemRewardQuantity==1,"Canonical Torben reward");
    for(unsigned Mask=0;Mask<64;++Mask)
    {
        const bool Accepted=Mask&1,Help=Mask&2,Bargain=Mask&4,Active=Mask&8,Completed=Mask&16,Claimed=Mask&32;
        const bool ValidEmpty=!Accepted && !Help && !Bargain && !Active && !Completed && !Claimed;
        const bool ValidActive=Accepted && Help!=Bargain && Active && !Completed && !Claimed;
        const bool ValidComplete=Accepted && Help!=Bargain && !Active && Completed && Claimed;
        Check(R::ValidateReceipt(Accepted,Help,Bargain,Active,Completed,Claimed)==(ValidEmpty||ValidActive||ValidComplete),"Receipt, choice and completion consistency");
    }
    for(unsigned StageIndex=0;StageIndex<5;++StageIndex)
    {
        const Stage Current=static_cast<Stage>(StageIndex);
        for(unsigned Mask=0;Mask<8;++Mask)
        {
            const bool ValidChoice=Mask&1,Duplicate=Mask&2,Mutable=Mask&4;
            Check(R::CanAccept(Current,ValidChoice,Duplicate,Mutable)==(Current==Stage::NotStarted && ValidChoice && !Duplicate && Mutable),"Accept only an unclaimed valid choice");
            Check(R::CanRecoverCargo(Current,ValidChoice,Duplicate,Mutable)==(Current==Stage::RecoverCargo && ValidChoice && !Duplicate && Mutable),"Cargo recovery respects stage and persistent receipt");
        }
        for(int Count=-3;Count<=4;++Count)for(unsigned Mask=0;Mask<16;++Mask)
        {
            const bool ValidID=Mask&1,Duplicate=Mask&2,Player=Mask&4,Mutable=Mask&8;
            const bool Expected=Current==Stage::DefeatBandits && Count>=0 && Count<2 && ValidID && !Duplicate && Player && Mutable;
            Check(R::CanCreditBandit(Current,ValidID,Duplicate,Count,Player,Mutable)==Expected,"Unique attributed player kills cannot advance outside the fight");
        }
        for(unsigned Mask=0;Mask<128;++Mask)
        {
            const bool Claimed=Mask&1,Mutable=Mask&2,Asset=Mask&4,Bag=Mask&8,Wallet=Mask&16,XP=Mask&32;
            const std::int64_t Gold=(Mask&64)?std::numeric_limits<std::int32_t>::max()-15:0;
            const bool Expected=Current==Stage::ReturnToTorben && !Claimed && Mutable && Asset && Bag && Wallet && XP && Gold==0;
            Check(R::CanTurnIn(Current,Claimed,Mutable,Asset,Bag,Wallet,XP,Gold)==Expected,"Turn-in preflight cannot partly pay without an asset, bag or wallet room");
        }
    }
    for(unsigned Mask=0;Mask<16;++Mask)for(int Kills=0;Kills<=2;++Kills)
    {
        const bool Active=Mask&1,Completed=Mask&2,Claimed=Mask&4,Cargo=Mask&8;
        const Stage Expected=(Completed||Claimed)?Stage::Completed:!Active?Stage::NotStarted:Kills<2?Stage::DefeatBandits:Cargo?Stage::ReturnToTorben:Stage::RecoverCargo;
        Check(R::ResolveStage(Active,Completed,Claimed,Kills,Cargo)==Expected,"Stage follows receipts, not cached transient state");
    }
    for(unsigned Mask=0;Mask<8;++Mask)for(int Kills=-1;Kills<=3;++Kills)for(int Sources=-1;Sources<=2;++Sources)
    {
        const bool Accepted=Mask&1,Cargo=Mask&2,Completed=Mask&4;
        const bool Empty=!Accepted && Kills==0 && !Cargo && Sources==0 && !Completed;
        const bool Fighting=Accepted && Kills>=0 && Kills<=2 && !Cargo && Sources==0 && !Completed;
        const bool Recovered=Accepted && Kills==2 && Cargo && Sources==1;
        Check(R::ValidateEncounterReceipts(Accepted,Kills,Cargo,Sources,Completed)==(Empty||Fighting||Recovered),"Cargo and kill receipt ledger rejects impossible states");
    }
    for(int Kills=0;Kills<=2;++Kills)for(int Cargo=0;Cargo<=1;++Cargo)
    {
        Check(R::ValidateActiveObjectiveMirror(2,Kills,1,Cargo,1,0,Kills,Cargo!=0),"Active objective mirror follows receipts");
        Check(!R::ValidateActiveObjectiveMirror(2,Kills,1,Cargo,1,1,Kills,Cargo!=0),"Failed return cannot retain transient completed return objective");
        Check(!R::ValidateActiveObjectiveMirror(2,Kills,1,Cargo,1,0,Kills+1,Cargo!=0),"Kill mirror cannot contradict receipts");
        Check(!R::ValidateActiveObjectiveMirror(2,Kills,1,Cargo,1,0,Kills,Cargo==0),"Cargo mirror cannot contradict receipt");
    }
    Check(!R::CanTurnIn(Stage::ReturnToTorben,false,true,true,true,true,true,-1),"Negative wallet rejected");
    Check(R::CanTurnIn(Stage::ReturnToTorben,false,true,true,true,true,true,std::numeric_limits<std::int32_t>::max()-16),"Exact int32 payout boundary accepted");
    Check(!R::CanTurnIn(Stage::ReturnToTorben,false,true,true,true,true,true,std::numeric_limits<std::int64_t>::max()),"Huge malformed wallet rejected");
    std::cout<<"Delivery rules: "<<Checks<<" checks, "<<Failures<<" failures\n";
    return Failures?1:0;
}
