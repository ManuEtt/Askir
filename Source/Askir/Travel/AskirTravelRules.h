#pragma once
#include <cmath>

/** Production rules shared by portal/arrival code and the portable test binary. */
namespace AskirTravelRules
{
struct Point { double X=0.0,Y=0.0,Z=0.0; };
inline bool Finite(const Point& Value)
{
    return std::isfinite(Value.X)&&std::isfinite(Value.Y)&&std::isfinite(Value.Z);
}
inline bool InRange(const Point& A,const Point& B,double Range)
{
    return Finite(A)&&Finite(B)&&std::isfinite(Range)&&Range>0.0
        &&std::hypot(A.X-B.X,A.Y-B.Y,A.Z-B.Z)<=Range;
}
inline bool OutsideExpandedTrigger(const Point& Candidate,const Point& Center,const Point& Extent,double Radius,double HalfHeight)
{
    if(!Finite(Candidate)||!Finite(Center)||!Finite(Extent)||Extent.X<0.0||Extent.Y<0.0||Extent.Z<0.0
        ||!std::isfinite(Radius)||!std::isfinite(HalfHeight)||Radius<=0.0||HalfHeight<Radius)return false;
    // Boundary contact is unsafe: begin-overlap can fire again on the next tick.
    return std::abs(Candidate.X-Center.X)>Extent.X+Radius
        ||std::abs(Candidate.Y-Center.Y)>Extent.Y+Radius
        ||std::abs(Candidate.Z-Center.Z)>Extent.Z+HalfHeight;
}
inline bool GroundSupports(double CapsuleBottom,double GroundHeight,double NormalZ,double WalkableNormalZ)
{
    return std::isfinite(CapsuleBottom)&&std::isfinite(GroundHeight)&&std::isfinite(NormalZ)
        &&std::isfinite(WalkableNormalZ)&&WalkableNormalZ>0.0&&WalkableNormalZ<=1.0
        &&NormalZ>=WalkableNormalZ&&NormalZ<=1.000001
        &&CapsuleBottom-GroundHeight>=-5.0&&CapsuleBottom-GroundHeight<=80.0;
}
inline bool CanQueue(bool HasPending,bool ValidDestination,bool ValidSnapshot)
{
    return !HasPending&&ValidDestination&&ValidSnapshot;
}
inline bool CanResumeLiveSource(bool SourceAuthoritative,bool SameWorld,bool WorldTearingDown,bool PlayerReady)
{
    // A matching map name cannot prove that the original actors still exist.
    return SourceAuthoritative&&SameWorld&&!WorldTearingDown&&PlayerReady;
}
inline bool ArrivalPortalMatches(bool ExplicitID,int MatchingPortals,bool Reciprocal)
{
    // Explicit arrivals never degrade to a PlayerStart when their portal is absent.
    return MatchingPortals==1&&(!ExplicitID||Reciprocal);
}
inline bool RetrySpawn(int Attempts,int MaximumAttempts=120)
{
    return Attempts>=0&&MaximumAttempts>0&&Attempts<MaximumAttempts;
}
}
