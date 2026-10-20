#include "Frontend/AskirFrontendRules.h"
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
int Checks=0;
void Expect(bool Actual,bool Expected,const char* Message)
{
    ++Checks;
    if(Actual!=Expected){std::cerr<<"Frontend rule failed: "<<Message<<'\n';std::exit(1);}
}
}
int main()
{
    using namespace AskirFrontendRules;
    // Enumerate every native readiness combination. An unready dependency must
    // never be compensated for by an unrelated valid dependency.
    for(unsigned Mask=0;Mask<64;++Mask)
    {
        StartReadiness State;
        State.ModeValid=(Mask&1)!=0;State.RuntimeReady=(Mask&2)!=0;
        State.PersistenceBusy=(Mask&4)!=0;State.FreshStartPending=(Mask&8)!=0;
        State.AuthoritativeTravelPending=(Mask&16)!=0;State.TargetWorldPackageValid=(Mask&32)!=0;
        const bool OnlyPreparedMask=Mask==35;
        Expect(CanPrepareStart(State),OnlyPreparedMask,"only a ready mode with a real target and no operation can prepare");
        for(int Active=0;Active<2;++Active)for(int Confirmed=0;Confirmed<2;++Confirmed)
            Expect(CanStartSession(CanPrepareStart(State),Active!=0,Confirmed!=0),
                OnlyPreparedMask&&!(Active==1&&Confirmed==0),"discard confirmation cannot bypass readiness");
    }
    for(unsigned Mask=0;Mask<1024;++Mask)
    {
        FreshWorldReadiness State;
        State.FreshStartPending=(Mask&1)!=0;State.HasSourceBackup=(Mask&2)!=0;
        State.ModeValid=(Mask&4)!=0;State.RuntimeReady=(Mask&8)!=0;
        State.AuthoritativeTravelPending=(Mask&16)!=0;State.WorldValid=(Mask&32)!=0;
        State.IsOriginalSourceWorld=(Mask&64)!=0;State.WorldTearingDown=(Mask&128)!=0;
        State.TargetMapMatches=(Mask&256)!=0;State.ControlledPlayerReady=(Mask&512)!=0;
        Expect(CanCommitFreshWorld(State),Mask==815,
            "only a confirmed distinct live target can clear the outgoing session ledgers");
    }
    for(unsigned Mask=0;Mask<16;++Mask)
        Expect(CanResumeLiveSource((Mask&1)!=0,(Mask&2)!=0,(Mask&4)!=0,(Mask&8)!=0),Mask==11,
            "a replacement same-name world cannot stand in for the captured source");
    for(unsigned Mask=0;Mask<16;++Mask)
        Expect(NeedsAutomaticRecovery((Mask&1)!=0,(Mask&2)!=0,(Mask&4)!=0,(Mask&8)!=0),Mask!=15,
            "missing recovery menu ownership triggers exactly one return handoff");
    double Deadline=-1.;
    Expect(MakeFreshStartDeadline(10.,Deadline),true,"valid monotonic handoff clock");
    Expect(Deadline==40.,true,"handoff is bounded to thirty seconds");
    Expect(FreshStartExpired(39.999,Deadline),false,"source is not cancelled while browse can still complete");
    Expect(FreshStartExpired(40.,Deadline),true,"exact timeout recovers the retained source");
    Expect(FreshStartExpired(41.,Deadline),true,"late ticks cannot extend the handoff indefinitely");
    Expect(FreshStartExpired(0.,0.),true,"uninitialized deadline cannot leave gameplay frozen");
    for(double Invalid:{-1.,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::max()})
    {
        const double Before=Deadline;
        Expect(MakeFreshStartDeadline(Invalid,Deadline),false,"invalid clocks cannot begin a new game transition");
        Expect(Deadline==Before,true,"rejected timeout does not overwrite the existing deadline");
        Expect(FreshStartExpired(Invalid,40.),true,"invalid current clock fails toward recovery");
    }
    for(double InvalidDeadline:{-1.,0.,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()})
        Expect(FreshStartExpired(10.,InvalidDeadline),true,"corrupt stored deadline fails toward recovery");
    std::cout<<"Frontend V81: "<<Checks<<" assertions passed\n";
}
