#include "Source/Askir/Magic/AskirSpellCatalog.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <string>
using namespace AskirMagicRules;
int main()
{
    int Checks=0;auto Check=[&](bool Condition){++Checks;assert(Condition);};
    std::set<std::string> IDs;int Schools[6]={},Effects[4]={};
    for(const Spell& S:Catalog)
    {
        Check(IDs.insert(S.ID).second);Check(std::strlen(S.Name)>0);++Schools[static_cast<int>(S.School)];++Effects[static_cast<int>(S.Kind)];
        Check(CheckCast(S,S.ManaCost,0,true,true,false,false)==Failure::None);
        Check(CheckCast(S,S.ManaCost-.01,0,true,true,false,false)==Failure::Mana);
        Check(CheckCast(S,100,.001,true,true,false,false)==Failure::Cooldown);
        Check(CheckCast(S,100,0,false,true,false,false)==Failure::Unavailable);
        Check(CheckCast(S,100,0,true,false,false,false)==Failure::Unavailable);
        Check(CheckCast(S,100,0,true,true,true,false)==Failure::Unavailable);
        Check(CheckCast(S,100,0,true,true,false,true)==Failure::Busy);
        Check(CheckCast(S,-1,0,true,true,false,false)==Failure::Invalid);
        Check(CheckCast(S,100,-1,true,true,false,false)==Failure::Invalid);
        Check(CheckCast(S,std::numeric_limits<double>::quiet_NaN(),0,true,true,false,false)==Failure::Invalid);
        Check(ValidDuration(S.Cooldown,S.Cooldown));Check(!ValidDuration(S.Cooldown+.01,S.Cooldown));
    }
    for(int Count:Schools){Check(Count==4);}
    for(int Count:Effects){Check(Count==6);}
    for(double Mana:{0.,10.,50.,100.,-2.,200.})
        for(double Delta:{0.,.01,1.,10.,1000.})
        {
            const double Result=Regenerate(Mana,100,7,Delta);Check(Result>=0&&Result<=100);
            if(Delta>0)Check(Result>=std::clamp(Mana,0.,100.));
        }
    Check(Regenerate(95,100,7,1)==100);Check(Regenerate(10,100,7,-1)==10);
    Check(Regenerate(std::numeric_limits<double>::quiet_NaN(),100,7,1)==7);
    Check(Regenerate(10,100,7,std::numeric_limits<double>::infinity())==10);
    for(double Damage:{0.,1.,15.,30.,50.,500.})
    {
        double Capacity=30;const double Remaining=ConsumeShield(Damage,Capacity,10,false);
        Check(std::abs(Remaining+30-Capacity-Damage)<.00001);Check(Capacity>=0);Check(Remaining>=0);
        Capacity=30;Check(ConsumeShield(Damage,Capacity,0,false)==Damage);Check(Capacity==30);
        Check(ConsumeShield(Damage,Capacity,10,true)==Damage);Check(Capacity==30);
    }
    Check(ValidMana(100,100));Check(!ValidMana(101,100));Check(!ValidMana(-1,100));
    Check(!ValidMana(std::numeric_limits<double>::infinity(),100));Check(!ValidDuration(std::numeric_limits<double>::quiet_NaN(),10));
    std::cout<<"PASS: "<<Checks<<" magic production-rule assertions; 24 canonical spells. Unreal runtime NOT_EXECUTED.\n";
}
