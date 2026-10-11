#include "Combat/AskirCombatComponent.h"
UAskirCombatComponent::UAskirCombatComponent(){PrimaryComponentTick.bCanEverTick=false;}
float UAskirCombatComponent::ApplyDamage(const FAskirDamageInfo& Info)
{
    if(bDead)return 0;
    float D=FMath::Max(0.f,Info.Damage);
    const bool Magic=Info.DamageType==EAskirDamageType::Magic||Info.DamageType==EAskirDamageType::Fire||Info.DamageType==EAskirDamageType::Frost||Info.DamageType==EAskirDamageType::Lightning||Info.DamageType==EAskirDamageType::Poison;
    if(Info.DamageType!=EAskirDamageType::TrueDamage)D*=1.f-FMath::Clamp(Magic?MagicResistance:PhysicalResistance,0.f,.95f);
    Health=FMath::Clamp(Health-D,0.f,MaxHealth);
    FAskirDamageInfo Applied=Info; Applied.Damage=D; OnDamageTaken.Broadcast(Applied);
    if(Health<=0)Kill(); return D;
}
void UAskirCombatComponent::Heal(float A){if(!bDead&&A>0)Health=FMath::Clamp(Health+A,0.f,MaxHealth);}
void UAskirCombatComponent::Kill(){if(bDead)return;bDead=true;Health=0;OnDeath.Broadcast();}
void UAskirCombatComponent::ResetHealth(){bDead=false;Health=MaxHealth;}
float UAskirCombatComponent::GetHealthPercent()const{return MaxHealth>0?Health/MaxHealth:0;}
void UAskirCombatComponent::RestoreHealth(float SavedHealth)
{
    Health = FMath::Clamp(SavedHealth, 0.f, MaxHealth);
    bDead = Health <= 0.f;
    if (bDead) OnDeath.Broadcast();
}
