#pragma once
#include <cmath>
#include <algorithm>

namespace AskirMagicRules
{
enum class Element { Fire, Ice, Storm, Earth, Light, Shadow };
enum class Effect { Projectile, Shield, MeleeArc, Area };
struct Spell { const char* ID; const char* Name; Element School; Effect Kind; double ManaCost; double Cooldown; double Power; };
enum class Failure { None, Invalid, Unavailable, Busy, Cooldown, Mana };
inline Failure CheckCast(const Spell& S,double Mana,double Remaining,bool Alive,bool Enabled,bool Menu,bool Busy)
{
    if(!std::isfinite(Mana)||Mana<0||!std::isfinite(Remaining)||Remaining<0||!std::isfinite(S.ManaCost)||S.ManaCost<=0||!std::isfinite(S.Cooldown)||S.Cooldown<=0||!std::isfinite(S.Power)||S.Power<=0)return Failure::Invalid;
    if(!Alive||!Enabled||Menu)return Failure::Unavailable;
    if(Busy)return Failure::Busy;
    if(Remaining>0)return Failure::Cooldown;
    if(Mana<S.ManaCost)return Failure::Mana;
    return Failure::None;
}
inline double Regenerate(double Mana,double Maximum,double Rate,double Delta)
{
    if(!std::isfinite(Maximum)||Maximum<=0)return 0;
    if(!std::isfinite(Mana))Mana=0;
    Mana=std::clamp(Mana,0.0,Maximum);
    if(!std::isfinite(Rate)||Rate<=0||!std::isfinite(Delta)||Delta<=0)return Mana;
    return std::min(Maximum,Mana+std::min(Delta,60.0)*std::min(Rate,Maximum));
}
inline double ConsumeShield(double Damage,double& Capacity,double Remaining,bool Bypass)
{
    if(!std::isfinite(Damage)||Damage<=0)return 0;
    if(Bypass||!std::isfinite(Capacity)||Capacity<=0||!std::isfinite(Remaining)||Remaining<=0)return Damage;
    const double Absorbed=std::min(Damage,Capacity);
    Capacity-=Absorbed;
    return Damage-Absorbed;
}
inline bool ValidMana(double Mana,double Maximum){return std::isfinite(Mana)&&std::isfinite(Maximum)&&Maximum>0&&Mana>=0&&Mana<=Maximum;}
inline bool ValidDuration(double Remaining,double Maximum){return std::isfinite(Remaining)&&Remaining>=0&&Remaining<=Maximum+.001;}
}
