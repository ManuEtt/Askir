#pragma once
#include <algorithm>
#include <cmath>

/** Engine-independent rules shared by targeting, melee and spell damage. */
namespace AskirCombatTargetRules
{
struct Participant
{
    bool Valid=false;
    bool PlayerControlled=false;
    bool HostileNPC=false;
    bool Dead=false;
    bool DamageEnabled=false;
    bool EncounterActive=true;
    bool TravelPending=false;
};

inline bool IsAvailable(const Participant& Actor)
{
    return Actor.Valid && !Actor.Dead && Actor.DamageEnabled && Actor.EncounterActive && !Actor.TravelPending;
}

inline bool CanDamage(const Participant& Attacker,const Participant& Target,bool SameWorld,bool SameActor=false)
{
    if(!SameWorld || SameActor || !IsAvailable(Attacker) || !IsAvailable(Target))return false;
    // A possessed NPC follows the player role. Friendly fire is never inferred from its class.
    if(Attacker.PlayerControlled)return Target.HostileNPC && !Target.PlayerControlled;
    return Attacker.HostileNPC && Target.PlayerControlled;
}

struct Vector {double X=0.0,Y=0.0,Z=0.0;};
inline bool IsFinite(const Vector& Value)
{
    return std::isfinite(Value.X) && std::isfinite(Value.Y) && std::isfinite(Value.Z);
}
inline bool IsValidHalfAngle(double Degrees)
{
    return std::isfinite(Degrees) && Degrees>=0.0 && Degrees<=90.0;
}

/** Normalize after scaling, avoiding overflow even for extreme finite coordinates. */
inline bool HorizontalUnit(const Vector& Value,double& X,double& Y)
{
    if(!IsFinite(Value))return false;
    const double Scale=std::max(std::abs(Value.X),std::abs(Value.Y));
    if(Scale<=1.e-8)return false;
    X=Value.X/Scale;Y=Value.Y/Scale;
    const double Length=std::sqrt(X*X+Y*Y);
    X/=Length;Y/=Length;return true;
}
inline bool HasHorizontalDirection(const Vector& Value)
{
    double X=0.0,Y=0.0;return HorizontalUnit(Value,X,Y);
}

/** DirectionToSource is defender -> source; elevation does not widen the facing cone. */
inline bool IsInsideFrontalCone(const Vector& Forward,const Vector& DirectionToSource,double HalfAngleDegrees)
{
    if(!IsValidHalfAngle(HalfAngleDegrees))return false;
    double ForwardX=0.0,ForwardY=0.0,SourceX=0.0,SourceY=0.0;
    if(!HorizontalUnit(Forward,ForwardX,ForwardY) || !HorizontalUnit(DirectionToSource,SourceX,SourceY))return false;
    constexpr double RadiansPerDegree=0.017453292519943295769;
    const double Dot=ForwardX*SourceX+ForwardY*SourceY;
    return Dot+1.e-12>=std::cos(HalfAngleDegrees*RadiansPerDegree);
}

/** HitDirection points attacker -> defender. Unknown hits need a reliable source vector.
 * Malformed incoming directions fail closed, even when a fallback source exists. */
inline bool IsFrontalBlock(const Vector& Forward,const Vector& HitDirection,
    const Vector& DirectionToSource,bool HasSource,double HalfAngleDegrees,bool TrueDamage)
{
    if(TrueDamage || !IsValidHalfAngle(HalfAngleDegrees) || !IsFinite(HitDirection))return false;
    if(HasHorizontalDirection(HitDirection))
        return IsInsideFrontalCone(Forward,{-HitDirection.X,-HitDirection.Y,-HitDirection.Z},HalfAngleDegrees);
    return HasSource && IsInsideFrontalCone(Forward,DirectionToSource,HalfAngleDegrees);
}
}
