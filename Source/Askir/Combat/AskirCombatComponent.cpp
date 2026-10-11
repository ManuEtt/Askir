#include "Combat/AskirCombatComponent.h"
#include "Combat/AskirGuardComponent.h"
#include "Engine/World.h"
UAskirCombatComponent::UAskirCombatComponent(){PrimaryComponentTick.bCanEverTick=false;}
float UAskirCombatComponent::ApplyDamage(const FAskirDamageInfo& Info)
{
    if(bDead || IsDodgeInvulnerable())return 0;
    float D=FMath::Max(0.f,Info.Damage);
    const bool Magic=Info.DamageType==EAskirDamageType::Magic||Info.DamageType==EAskirDamageType::Fire||Info.DamageType==EAskirDamageType::Frost||Info.DamageType==EAskirDamageType::Lightning||Info.DamageType==EAskirDamageType::Poison;
    if(Info.DamageType!=EAskirDamageType::TrueDamage)D*=1.f-FMath::Clamp(Magic?MagicResistance:PhysicalResistance,0.f,.95f);
    if(GetOwner())
    {
        if(UAskirGuardComponent* Guard=GetOwner()->FindComponentByClass<UAskirGuardComponent>())
            D=Guard->AbsorbIncomingDamage(D);
    }
    if(D>0.f)
    {
        LastDamageCauser=Info.DamageCauser.Get();
        LastDamageTimeSeconds=GetWorld()?GetWorld()->GetTimeSeconds():-1.0;
    }
    Health=FMath::Clamp(Health-D,0.f,MaxHealth);
    FAskirDamageInfo Applied=Info; Applied.Damage=D; OnDamageTaken.Broadcast(Applied);
    if(Health<=0)Kill(); return D;
}
void UAskirCombatComponent::Heal(float A){if(!bDead&&A>0)Health=FMath::Clamp(Health+A,0.f,MaxHealth);}
void UAskirCombatComponent::Kill(){if(bDead)return;bDead=true;Health=0;OnDeath.Broadcast();}
void UAskirCombatComponent::ResetHealth(){bDead=false;Health=MaxHealth;DodgeInvulnerableUntil=-1.f;ClearDamageAttribution();}
float UAskirCombatComponent::GetHealthPercent()const{return MaxHealth>0?Health/MaxHealth:0;}
void UAskirCombatComponent::RestoreHealth(float SavedHealth)
{
    Health = FMath::Clamp(SavedHealth, 0.f, MaxHealth);
    bDead = Health <= 0.f;
    // Loading a saved corpse must never replay gameplay death/loot/quest delegates.
    ClearDamageAttribution();
}

void UAskirCombatComponent::GrantDodgeInvulnerability(float Duration)
{
    if (!bDead && GetWorld() && Duration > 0.f)
        DodgeInvulnerableUntil = FMath::Max(DodgeInvulnerableUntil, GetWorld()->GetTimeSeconds() + FMath::Clamp(Duration, 0.f, 2.f));
}
bool UAskirCombatComponent::IsDodgeInvulnerable() const
{
    return GetWorld() && GetWorld()->GetTimeSeconds() < DodgeInvulnerableUntil;
}

AActor* UAskirCombatComponent::GetLastDamageCauser() const
{
    const UWorld* World=GetWorld();
    if(!World || LastDamageTimeSeconds<0.0 || KillCreditWindowSeconds<=0.f) return nullptr;
    const double Elapsed=World->GetTimeSeconds()-LastDamageTimeSeconds;
    if(Elapsed<0.0 || Elapsed>static_cast<double>(KillCreditWindowSeconds)) return nullptr;
    return LastDamageCauser.Get();
}
void UAskirCombatComponent::ClearDamageAttribution()
{
    LastDamageCauser.Reset();
    LastDamageTimeSeconds=-1.0;
}
