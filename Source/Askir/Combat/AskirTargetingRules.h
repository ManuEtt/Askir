#pragma once
#include <algorithm>
#include <cmath>
#include <limits>

/** Geometry and ordering shared by runtime targeting and the portable tests. */
namespace AskirTargetingRules
{
constexpr double Pi=3.14159265358979323846;
constexpr double TwoPi=2.*Pi;
struct Vector { double X=0.,Y=0.,Z=0.; };
struct Metrics
{
    double Distance=0.;
    double Alignment=0.;
    double Yaw=0.;
    double Score=0.;
};
inline bool Finite(const Vector& V)
{
    return std::isfinite(V.X)&&std::isfinite(V.Y)&&std::isfinite(V.Z);
}
inline bool Normalize(const Vector& V,Vector& Out,double& Length)
{
    if(!Finite(V))return false;
    const double Magnitude=std::hypot(V.X,V.Y,V.Z);
    if(!std::isfinite(Magnitude)||Magnitude<=1.e-8)return false;
    const Vector Unit{V.X/Magnitude,V.Y/Magnitude,V.Z/Magnitude};
    if(!Finite(Unit))return false;
    Out=Unit;Length=Magnitude;return true;
}
inline bool Direction(const Vector& Origin,const Vector& Target,Vector& Out,double& Distance)
{
    return Finite(Origin)&&Finite(Target)&&Normalize({Target.X-Origin.X,Target.Y-Origin.Y,Target.Z-Origin.Z},Out,Distance);
}
inline bool InRange(double Distance,double Radius)
{
    return std::isfinite(Distance)&&Distance>1.e-8&&std::isfinite(Radius)&&Radius>0.&&Distance<=Radius;
}
inline bool Evaluate(const Vector& Origin,const Vector& Forward,const Vector& Target,
    double Radius,double MinimumAlignment,Metrics& Out)
{
    Vector Aim,View;double Distance=0.,ViewLength=0.;
    if(!std::isfinite(MinimumAlignment)||MinimumAlignment< -1.||MinimumAlignment>1.||
        !Direction(Origin,Target,Aim,Distance)||!InRange(Distance,Radius)||!Normalize(Forward,View,ViewLength))return false;
    const double Alignment=std::clamp(Aim.X*View.X+Aim.Y*View.Y+Aim.Z*View.Z,-1.,1.);
    if(Alignment<MinimumAlignment)return false;
    // A vertical view has no useful left/right bearing. The native caller supplies
    // a horizontal fallback, but reject it here rather than inventing a direction.
    const double HorizontalView=std::hypot(View.X,View.Y);
    if(HorizontalView<=1.e-8)return false;
    const double Cross=View.X*Aim.Y-View.Y*Aim.X;
    const double Dot=View.X*Aim.X+View.Y*Aim.Y;
    const double Yaw=std::atan2(Cross,Dot);
    Out={Distance,Alignment,Yaw,.75*(1.-Alignment)*.5+.25*Distance/Radius};
    return true;
}
/** Negative means A is preferred. Equal geometry is resolved by stable actor path in UE. */
inline int CompareRank(const Metrics& A,const Metrics& B)
{
    const bool AValid=std::isfinite(A.Score)&&std::isfinite(A.Distance)&&A.Distance>0.;
    const bool BValid=std::isfinite(B.Score)&&std::isfinite(B.Distance)&&B.Distance>0.;
    if(AValid!=BValid)return AValid?-1:1;
    if(!AValid)return 0;
    if(A.Score!=B.Score)return A.Score<B.Score?-1:1;
    if(A.Distance!=B.Distance)return A.Distance<B.Distance?-1:1;
    return 0;
}
/** Positive direction is the camera's right. The current actor is excluded by the caller. */
inline double CycleOffset(double CandidateYaw,double CurrentYaw,int DirectionSign)
{
    if(!std::isfinite(CandidateYaw)||!std::isfinite(CurrentYaw)||DirectionSign==0)
        return std::numeric_limits<double>::infinity();
    double Delta=std::fmod(CandidateYaw-CurrentYaw,TwoPi);
    if(!std::isfinite(Delta))return std::numeric_limits<double>::infinity();
    if(DirectionSign<0)Delta=-Delta;
    if(Delta<0.)Delta+=TwoPi;
    return Delta==0.?0.:Delta;
}
inline bool OcclusionExpired(double Now,double LastVisible,double Grace)
{
    if(!std::isfinite(Now)||!std::isfinite(LastVisible)||!std::isfinite(Grace)||
        Now<0.||LastVisible<0.||Now<LastVisible||Grace<0.)return true;
    return Now-LastVisible>=Grace;
}
}
