#include "Source/Askir/Combat/AskirCombatTargetRules.h"
#include <cstdint>
#include <iostream>
#include <limits>

namespace R=AskirCombatTargetRules;
std::uint64_t Checks=0,Failures=0;
void Check(bool Result,const char* Description)
{
    ++Checks;if(!Result){++Failures;std::cerr<<"FAIL: "<<Description<<'\n';}
}
R::Participant Decode(unsigned Mask)
{
    R::Participant Actor;
    Actor.Valid=(Mask&1)!=0;Actor.PlayerControlled=(Mask&2)!=0;Actor.HostileNPC=(Mask&4)!=0;
    Actor.Dead=(Mask&8)!=0;Actor.DamageEnabled=(Mask&16)!=0;
    Actor.EncounterActive=(Mask&32)!=0;Actor.TravelPending=(Mask&64)!=0;return Actor;
}
R::Vector Heading(double Degrees,double Magnitude=1.0,double Height=0.0)
{
    constexpr double ToRadians=0.017453292519943295769;
    return {std::cos(Degrees*ToRadians)*Magnitude,std::sin(Degrees*ToRadians)*Magnitude,Height};
}
int main()
{
    // Every combination of availability, role, world identity and actor identity.
    for(unsigned SourceMask=0;SourceMask<128;++SourceMask)for(unsigned TargetMask=0;TargetMask<128;++TargetMask)
        for(unsigned Context=0;Context<4;++Context)
        {
            const auto Source=Decode(SourceMask),Target=Decode(TargetMask);
            const bool SameWorld=(Context&1)!=0,SameActor=(Context&2)!=0;
            bool Expected=SameWorld && !SameActor;
            for(const auto& Actor:{Source,Target})
                if(!Actor.Valid || Actor.Dead || !Actor.DamageEnabled || !Actor.EncounterActive || Actor.TravelPending)Expected=false;
            const bool PlayerAttacksHostile=Source.PlayerControlled && Target.HostileNPC && !Target.PlayerControlled;
            const bool HostileAttacksPlayer=!Source.PlayerControlled && Source.HostileNPC && Target.PlayerControlled;
            Check(R::CanDamage(Source,Target,SameWorld,SameActor)==(Expected && (PlayerAttacksHostile||HostileAttacksPlayer)),"Full participant and identity policy");
        }
    const R::Vector Forward{1.0,0.0,0.0},FrontHit{-1.0,0.0,0.0},RearHit{1.0,0.0,0.0},Zero{};
    Check(R::IsFrontalBlock(Forward,FrontHit,Zero,false,65.0,false),"Incoming direction is inverted to point toward source");
    Check(!R::IsFrontalBlock(Forward,RearHit,{1,0,0},true,65.0,false),"Reliable rear hit overrides contradictory source fallback");
    Check(R::IsFrontalBlock(Forward,FrontHit,{-1,0,0},true,65.0,false),"Reliable front hit overrides contradictory source fallback");
    Check(!R::IsFrontalBlock(Forward,FrontHit,Zero,false,65.0,true),"True damage bypasses directional guard");
    Check(!R::IsFrontalBlock(Forward,Zero,Zero,false,65.0,false),"Unknown direction without causer cannot grant guard");
    Check(R::IsFrontalBlock(Forward,Zero,{10,0,30},true,65.0,false),"Known front causer handles missing hit direction");
    Check(!R::IsFrontalBlock(Forward,Zero,{-10,0,30},true,65.0,false),"Known rear causer never grants guard");
    Check(!R::IsFrontalBlock(Forward,Zero,{0,0,30},true,65.0,false),"Vertical-only source has no horizontal bearing");
    Check(R::IsFrontalBlock(Forward,{0,0,-100},{1,0,0},true,65.0,false),"Vertical-only incoming direction may use valid source bearing");

    // Known angular differences provide an independent expected result for arbitrary facing.
    for(int Facing=-180;Facing<=180;Facing+=15)for(int Offset=-180;Offset<=180;Offset+=5)
        for(int HalfAngle=0;HalfAngle<=90;HalfAngle+=5)
        {
            const auto DefenderForward=Heading(Facing,5.0,10.0);
            const auto SourceBearing=Heading(Facing+Offset,100.0,-400.0);
            const R::Vector Incoming{-SourceBearing.X,-SourceBearing.Y,-SourceBearing.Z};
            const bool Expected=std::abs(Offset)<=HalfAngle;
            Check(R::IsInsideFrontalCone(DefenderForward,SourceBearing,HalfAngle)==Expected,"Cone includes boundary and rotates with defender");
            Check(R::IsFrontalBlock(DefenderForward,Incoming,Zero,false,HalfAngle,false)==Expected,"Incoming direction uses the same facing cone");
            Check(R::IsFrontalBlock(DefenderForward,Zero,SourceBearing,true,HalfAngle,false)==Expected,"Source fallback uses the same facing cone");
        }
    const double Invalid[]={std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity()};
    for(double Value:Invalid)
    {
        Check(!R::IsFrontalBlock(Forward,FrontHit,Zero,false,Value,false),"Nonfinite angle cannot grant guard");
        for(int Axis=0;Axis<3;++Axis)
        {
            R::Vector BadIncoming=FrontHit,BadSource{1,0,0},BadForward=Forward;
            if(Axis==0){BadIncoming.X=Value;BadSource.X=Value;BadForward.X=Value;}
            if(Axis==1){BadIncoming.Y=Value;BadSource.Y=Value;BadForward.Y=Value;}
            if(Axis==2){BadIncoming.Z=Value;BadSource.Z=Value;BadForward.Z=Value;}
            Check(!R::IsFrontalBlock(Forward,BadIncoming,{1,0,0},true,65.0,false),"Malformed hit direction cannot be rescued by source fallback");
            Check(!R::IsFrontalBlock(BadForward,FrontHit,Zero,false,65.0,false),"Malformed facing cannot grant guard");
            Check(!R::IsFrontalBlock(Forward,Zero,BadSource,true,65.0,false),"Malformed source bearing cannot grant guard");
        }
    }
    for(double Angle:{-1000.0,-1.0,-.001,90.001,91.0,180.0,1000.0})
        Check(!R::IsFrontalBlock(Forward,FrontHit,Zero,false,Angle,false),"Invalid half-angle range fails closed");
    const double Large=std::numeric_limits<double>::max();
    Check(R::IsInsideFrontalCone({Large,Large,Large},{Large,Large,-Large},0.0),"Large finite vectors normalize without overflow");
    Check(!R::IsInsideFrontalCone({Large,Large,Large},{-Large,-Large,-Large},90.0),"Large opposite vectors remain behind");
    Check(!R::IsFrontalBlock(Zero,FrontHit,Zero,false,65.0,false),"Zero facing cannot grant guard");
    Check(!R::IsFrontalBlock(Forward,Zero,{1.e-10,0,0},true,65.0,false),"Coincident source position is not reliable");
    Check(R::IsFrontalBlock(Forward,{0,-1,0},Zero,false,90.0,false),"Ninety-degree boundary remains inside the permitted cone");
    std::cout<<Checks<<" combat target checks, "<<Failures<<" failures\n";return Failures?1:0;
}
