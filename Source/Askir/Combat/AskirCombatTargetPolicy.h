#pragma once
#include "CoreMinimal.h"

class AActor;
struct FAskirDamageInfo;

/** Gameplay attack eligibility. Low-level ApplyDamage intentionally remains available to scripts. */
namespace AskirCombatTargetPolicy
{
    ASKIR_API bool CanDamage(const AActor* Attacker,const AActor* Target);
    /** A horizontal facing cone. Incoming HitDirection points attacker -> defender. */
    ASKIR_API bool IsFrontalBlock(const AActor* Defender,const FAskirDamageInfo& Damage,float HalfAngleDegrees=65.f);
}
