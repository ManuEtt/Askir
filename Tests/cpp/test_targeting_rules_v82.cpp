#include "Combat/AskirTargetingRules.h"
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

namespace
{
int Checks=0;
void Expect(bool Result,const char* Message)
{
    ++Checks;if(!Result){std::cerr<<"Targeting rule failed: "<<Message<<'\n';std::exit(1);}
}
bool Near(double A,double B,double Epsilon=1.e-10) { return std::abs(A-B)<=Epsilon; }
}
int main()
{
    using namespace AskirTargetingRules;
    const Vector Origin{0.,0.,0.},Forward{1.,0.,0.};
    Vector Unit{9.,8.,7.};double Distance=123.;
    Expect(Direction(Origin,{3.,4.,12.},Unit,Distance),"three-dimensional aim is defined");
    Expect(Near(Distance,13.)&&Near(Unit.X,3./13.)&&Near(Unit.Y,4./13.)&&Near(Unit.Z,12./13.),"aim retains height and unit length");
    const Vector SavedUnit=Unit;const double SavedDistance=Distance;
    Expect(!Direction(Origin,Origin,Unit,Distance),"coincident target cannot create an aim vector");
    Expect(Unit.X==SavedUnit.X&&Distance==SavedDistance,"failed direction does not overwrite caller state");
    const double NaN=std::numeric_limits<double>::quiet_NaN();
    const double Infinity=std::numeric_limits<double>::infinity();
    const double Largest=std::numeric_limits<double>::max();
    for(double Bad:{NaN,Infinity,-Infinity})for(int Axis=0;Axis<3;++Axis)
    {
        Vector Broken{1.,2.,3.};if(Axis==0)Broken.X=Bad;else if(Axis==1)Broken.Y=Bad;else Broken.Z=Bad;
        Expect(!Direction(Origin,Broken,Unit,Distance),"nonfinite targets rejected on every axis");
        Expect(!Direction(Broken,Forward,Unit,Distance),"nonfinite origins rejected on every axis");
        Metrics M;Expect(!Evaluate(Origin,Broken,Forward,1400.,0.,M),"nonfinite view rejected on every axis");
    }
    Expect(!Direction({-Largest,0.,0.},{Largest,0.,0.},Unit,Distance),"overflowing difference cannot produce false finite direction");
    Expect(Direction(Origin,{Largest/4.,Largest/4.,Largest/4.},Unit,Distance),"hypot supports finite large directions without squared overflow");
    Metrics M;
    Expect(!Evaluate(Origin,Origin,{1.,0.,0.},1400.,0.,M),"zero camera vector is rejected");
    Expect(!Evaluate(Origin,{0.,0.,1.},{1.,0.,0.},1400.,-1.,M),"vertical view needs an explicit horizontal caller fallback");
    for(double Radius:{0.,-1.,NaN,Infinity,-Infinity})
        Expect(!Evaluate(Origin,Forward,{1.,0.,0.},Radius,-1.,M),"invalid radius rejected");
    for(double Minimum:{-1.01,1.01,NaN,Infinity,-Infinity})
        Expect(!Evaluate(Origin,Forward,{1.,0.,0.},1400.,Minimum,M),"invalid cone rejected");
    Expect(Evaluate(Origin,Forward,{1400.,0.,0.},1400.,1.,M),"exact range and cone endpoints accepted");
    Expect(!Evaluate(Origin,Forward,{1400.0001,0.,0.},1400.,1.,M),"just outside range rejected");
    Expect(!Evaluate(Origin,Forward,{1.,.001,0.},1400.,1.,M),"outside zero-width cone rejected");
    Expect(Evaluate(Origin,Forward,{-10.,0.,0.},1400.,-1.,M),"retention geometry supports a rear target");
    Expect(Near(M.Yaw,Pi)&&Near(M.Alignment,-1.),"rear bearing and alignment are deterministic");
    // Sample the entire surrounding circle at multiple distances. Acquisition
    // only admits the forward 75 degree cone and the inclusive 1400 cm radius.
    const double Threshold=std::cos(75.*Pi/180.);
    for(int Degrees=-180;Degrees<180;Degrees+=5)for(int Range=50;Range<=2000;Range+=50)
    {
        const double Angle=Degrees*Pi/180.;
        const Vector Target{Range*std::cos(Angle),Range*std::sin(Angle),0.};
        const bool Valid=Evaluate(Origin,Forward,Target,1400.,Threshold,M);
        const bool Boundary=std::abs(Degrees)==75||Range==1400;
        if(!Boundary)Expect(Valid==(std::abs(Degrees)<75&&Range<1400),"full-circle acquisition rejects rear, wide-angle and distant targets");
    }
    // Direct interior/exterior checks avoid relying on trig rounding at the cone edge.
    for(int Degrees=-174;Degrees<175;Degrees+=3)for(int Range=100;Range<=1900;Range+=100)
    {
        const double Angle=Degrees*Pi/180.;const Vector Target{Range*std::cos(Angle),Range*std::sin(Angle),0.};
        const bool Actual=Evaluate(Origin,Forward,Target,1400.000001,Threshold,M);
        const bool Expected=std::abs(Degrees)<=75&&Range<=1400;
        if(std::abs(Degrees)!=75)Expect(Actual==Expected,"cone/radius filter matches independent angular grid");
        if(Actual)
        {
            Expect(Near(M.Distance,Range,1.e-8),"rank uses actual target distance");
            Expect(Near(M.Yaw,Angle),"cycle bearing follows camera-relative left/right");
            Expect(M.Score>=0.&&M.Score<=1.,"rank has a finite bounded score");
        }
    }
    Metrics NearCenter,FarCenter,Side;
    Expect(Evaluate(Origin,Forward,{300.,0.,0.},1400.,Threshold,NearCenter),"center target ranked");
    Expect(Evaluate(Origin,Forward,{900.,0.,0.},1400.,Threshold,FarCenter),"far center target ranked");
    Expect(Evaluate(Origin,Forward,{150.,259.8076211353316,0.},1400.,Threshold,Side),"side target ranked");
    Expect(CompareRank(NearCenter,FarCenter)<0,"same angle prefers closer enemy");
    Expect(CompareRank(NearCenter,Side)<0,"same distance prefers camera center");
    Expect(CompareRank(NearCenter,NearCenter)==0,"exact geometry tie is delegated to stable actor path");
    // Equal geometry must compare equally so the native caller can apply its stable actor-path tie-break.
    Metrics EqualGeometry=NearCenter;
    Expect(CompareRank(NearCenter,EqualGeometry)==0&&CompareRank(EqualGeometry,NearCenter)==0,
        "equal geometry leaves ordering to stable actor path");
    Metrics SameScoreFarther=NearCenter;
    SameScoreFarther.Distance+=1.;
    Expect(CompareRank(NearCenter,SameScoreFarther)<0&&CompareRank(SameScoreFarther,NearCenter)>0,
        "equal scores prefer the nearer target in both comparison directions");
    Metrics InvalidDistance=NearCenter;InvalidDistance.Distance=0.;
    Expect(CompareRank(NearCenter,InvalidDistance)<0&&CompareRank(InvalidDistance,NearCenter)>0,
        "nonpositive target distance cannot win ranking");
    Metrics Invalid=NearCenter;Invalid.Score=NaN;
    Expect(CompareRank(NearCenter,Invalid)<0&&CompareRank(Invalid,NearCenter)>0,"invalid rank cannot beat a valid target");
    for(int A=0;A<72;++A)for(int B=0;B<72;++B)for(int DirectionSign:{-1,1})
    {
        const double Current=(-180.+A*5.)*Pi/180.;const double Candidate=(-180.+B*5.)*Pi/180.;
        const int Steps=DirectionSign>0?(B-A+72)%72:(A-B+72)%72;
        const double Offset=CycleOffset(Candidate,Current,DirectionSign);
        // Floating point can represent a full wrap for a mathematical zero only
        // when the two angles differ; identical input angles remain exact zero.
        Expect(Near(Offset,Steps*5.*Pi/180.),"directional cycling wraps consistently through every bearing pair");
        Expect(Offset>=0.&&Offset<TwoPi,"cycle offset is finite and bounded");
    }
    Expect(!std::isfinite(CycleOffset(NaN,0.,1)),"invalid candidate bearing rejected");
    Expect(!std::isfinite(CycleOffset(0.,Infinity,1)),"invalid current bearing rejected");
    Expect(!std::isfinite(CycleOffset(0.,0.,0)),"zero cycle direction cannot select an actor");
    Expect(!OcclusionExpired(.349,0.,.35),"brief cover retains target");
    Expect(OcclusionExpired(.35,0.,.35),"exact grace endpoint releases target");
    Expect(OcclusionExpired(.351,0.,.35),"late frame cannot extend cover grace");
    Expect(OcclusionExpired(0.,0.,0.),"zero grace releases on first occluded frame");
    for(double Bad:{NaN,Infinity,-Infinity,-1.})
    {
        Expect(OcclusionExpired(Bad,0.,.35),"invalid current clock fails toward release");
        Expect(OcclusionExpired(1.,Bad,.35),"invalid last-visible clock fails toward release");
        Expect(OcclusionExpired(1.,0.,Bad),"invalid grace fails toward release");
    }
    Expect(OcclusionExpired(1.,2.,.35),"backward clock cannot strand a lock");
    for(int Milliseconds=0;Milliseconds<=1000;++Milliseconds)
        Expect(OcclusionExpired(Milliseconds/1000.,0.,.35)==(Milliseconds>=350),"occlusion grace does not depend on tick frequency");
    std::cout<<"Targeting V82: "<<Checks<<" assertions passed\n";
}
