#include "Travel/AskirTravelRules.h"
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
int Checks=0;
void Check(bool Condition,const char* Message)
{
 ++Checks;
 if(!Condition){std::cerr<<"FAILED: "<<Message<<'\n';std::exit(1);}
}
}
int main()
{
 using namespace AskirTravelRules;
 const double NaN=std::numeric_limits<double>::quiet_NaN();
 const double Inf=std::numeric_limits<double>::infinity();
 const Point Origin{},Extent{120,220,220};
 Check(InRange(Origin,{280,0,0},280),"Interaction boundary is inclusive");
 Check(!InRange(Origin,{280.001,0,0},280),"Remote API invocation cannot bypass proximity");
 for(double Invalid:{NaN,Inf,-Inf})
 {
  Check(!InRange(Origin,{Invalid,0,0},280),"Nonfinite position denied");
  Check(!InRange(Origin,Origin,Invalid),"Nonfinite range denied");
  Check(!OutsideExpandedTrigger({0,420,0},Origin,{120,Invalid,220},42,96),"Invalid trigger cannot claim a safe landing");
  Check(!GroundSupports(Invalid,0,1,.71),"Invalid landing height denied");
  Check(!GroundSupports(2,0,Invalid,.71),"Invalid support normal denied");
 }
 Check(!InRange(Origin,Origin,0),"Zero interaction range denied");
 Check(!InRange(Origin,Origin,-1),"Negative interaction range denied");
 Check(!InRange({1.e308,0,0},{-1.e308,0,0},280),"Overflowing distance denied");
 Check(!OutsideExpandedTrigger({162,0,0},Origin,Extent,42,96),"Touching expanded trigger boundary is unsafe");
 Check(OutsideExpandedTrigger({162.01,0,0},Origin,Extent,42,96),"Full capsule outside horizontal trigger accepted");
 Check(!OutsideExpandedTrigger({0,0,316},Origin,Extent,42,96),"Capsule height participates in arrival safety");
 Check(OutsideExpandedTrigger({0,0,316.01},Origin,Extent,42,96),"Full capsule vertically clear accepted");
 Check(!OutsideExpandedTrigger({0,420,0},Origin,Extent,100,96),"Invalid capsule dimensions denied");
 Check(!OutsideExpandedTrigger({0,420,0},Origin,{-1,220,220},42,96),"Negative trigger dimensions denied");
 for(int X=-400;X<=400;++X)
 {
  Check(InRange(Origin,{double(X),0,0},280)==(X>=-280&&X<=280),"Portal range across signed axis");
  const bool Outside=OutsideExpandedTrigger({double(X),0,0},Origin,Extent,42,96);
  Check(Outside==(X<-162||X>162),"Expanded trigger accepts only full-capsule clearance");
  Check(Outside==OutsideExpandedTrigger({double(-X),0,0},Origin,Extent,42,96),"Arrival bounds symmetric");
 }
 for(int Flags=0;Flags<8;++Flags)
 {
  const bool Pending=(Flags&1)!=0,Destination=(Flags&2)!=0,Snapshot=(Flags&4)!=0;
  Check(CanQueue(Pending,Destination,Snapshot)==(!Pending&&Destination&&Snapshot),"Queue is a non-overwriting state transition");
 }
 for(int Flags=0;Flags<16;++Flags)
 {
  const bool Authoritative=(Flags&1)!=0,SameWorld=(Flags&2)!=0,TearingDown=(Flags&4)!=0,PlayerReady=(Flags&8)!=0;
  Check(CanResumeLiveSource(Authoritative,SameWorld,TearingDown,PlayerReady)==
    (Authoritative&&SameWorld&&!TearingDown&&PlayerReady),"Only intact original actors can replace the captured handoff");
 }
 Check(!CanResumeLiveSource(true,false,false,true),"Replacement world with same map name must retain snapshot");
 Check(!CanResumeLiveSource(true,true,true,true),"Original world already tearing down cannot resume gameplay");
 Check(GroundSupports(2,0,1,.71),"Grounded capsule accepted");
 Check(GroundSupports(-5,0,.71,.71),"Small tolerated ground penetration accepted before grounding");
 Check(!GroundSupports(-5.01,0,1,.71),"Buried capsule denied");
 Check(!GroundSupports(80.01,0,1,.71),"Unsupported airborne spawn denied");
 Check(!GroundSupports(2,0,.70,.71),"Steep floor denied");
 Check(!GroundSupports(2,0,-1,.71),"Ceiling is not ground");
 Check(!GroundSupports(2,0,1,0),"Invalid walkable threshold denied");
 Check(!ArrivalPortalMatches(true,0,true),"Missing explicit portal cannot fall back");
 Check(!ArrivalPortalMatches(true,2,true),"Duplicate portal IDs cannot choose first match");
 Check(!ArrivalPortalMatches(true,1,false),"One-way mismatched explicit arrival denied");
 Check(ArrivalPortalMatches(true,1,true),"Unique reciprocal explicit route accepted");
 Check(ArrivalPortalMatches(false,1,false),"Legacy unique reciprocal-map route accepted");
 Check(!ArrivalPortalMatches(false,2,true),"Ambiguous legacy arrival denied");
 Check(RetrySpawn(119),"Final retry allowed before timeout");
 Check(!RetrySpawn(120),"Timeout becomes recoverable failure");
 Check(!RetrySpawn(-1),"Invalid retry counter denied");
 std::cout<<Checks<<" travel assertions passed\n";
}
