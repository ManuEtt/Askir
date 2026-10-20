#include "Combat/AskirCombatComponent.h"
#include "Combat/AskirGuardComponent.h"
#include "Combat/AskirCombatTargetPolicy.h"
#include "Magic/AskirMagicComponent.h"
#include "Characters/AskirCharacter.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
UAskirCombatComponent::UAskirCombatComponent(){PrimaryComponentTick.bCanEverTick=false;}
float UAskirCombatComponent::ApplyDamage(const FAskirDamageInfo& Info,bool bSkipLegacyGuard)
{
    if(const auto* Character=Cast<AAskirCharacter>(GetOwner()))
        if(Character->IsPlayerControlled()&&Character->IsWorldTravelPending())return 0.f;
    if(bDead || !bDamageEnabled || IsDodgeInvulnerable() || !FMath::IsFinite(Info.Damage))return 0;
    float D=FMath::Max(0.f,Info.Damage);
    const bool Magic=Info.DamageType==EAskirDamageType::Magic||Info.DamageType==EAskirDamageType::Fire||Info.DamageType==EAskirDamageType::Frost||Info.DamageType==EAskirDamageType::Lightning||Info.DamageType==EAskirDamageType::Poison;
    if(Info.DamageType!=EAskirDamageType::TrueDamage)D*=1.f-FMath::Clamp(Magic?MagicResistance:PhysicalResistance,0.f,.95f);
    if(GetOwner())
    {
        if(UAskirMagicComponent* Shield=GetOwner()->FindComponentByClass<UAskirMagicComponent>())D=Shield->AbsorbIncomingDamage(D,Info.DamageType==EAskirDamageType::TrueDamage);
        // Native character blocking has already paid its stamina cost. Legacy
        // guard components may protect other actors, but never stack both paths
        // or absorb a rear/unknown-direction/true-damage hit.
        const auto* Character=Cast<AAskirCharacter>(GetOwner());
        if(!bSkipLegacyGuard&&(!Character||!Character->IsBlocking())&&AskirCombatTargetPolicy::IsFrontalBlock(GetOwner(),Info))
            if(UAskirGuardComponent* Guard=GetOwner()->FindComponentByClass<UAskirGuardComponent>())D=Guard->AbsorbIncomingDamage(D);
    }
    // No delegate, stagger or attribution update for zero/fully blocked hits.
    if(D<=0.f)return 0.f;
    const float AppliedDamage=FMath::Min(D,Health);
    LastDamageCauser=ResolveDamageRecipient(Info.DamageCauser.Get());
    LastDamageTimeSeconds=GetWorld()?GetWorld()->GetTimeSeconds():-1.0;
    Health=FMath::Clamp(Health-D,0.f,MaxHealth);
    FAskirDamageInfo Applied=Info; Applied.Damage=AppliedDamage; OnDamageTaken.Broadcast(Applied);
    if(Health<=0)Kill(); return AppliedDamage;
}
void UAskirCombatComponent::Heal(float A){if(!bDead&&FMath::IsFinite(A)&&A>0)Health=FMath::Clamp(Health+A,0.f,MaxHealth);}
void UAskirCombatComponent::Kill(){if(bDead)return;bDead=true;Health=0;OnDeath.Broadcast();}
void UAskirCombatComponent::ResetHealth(){bDead=false;Health=MaxHealth;DodgeInvulnerableUntil=-1.f;ClearDamageAttribution();}
float UAskirCombatComponent::GetHealthPercent()const{return MaxHealth>0?Health/MaxHealth:0;}
void UAskirCombatComponent::RestoreHealth(float SavedHealth)
{
    Health = FMath::IsFinite(SavedHealth) ? FMath::Clamp(SavedHealth, 0.f, MaxHealth) : MaxHealth;
    bDead = Health <= 0.f;
    DodgeInvulnerableUntil=-1.f;
    // Loading a saved corpse must never replay gameplay death/loot/quest delegates.
    ClearDamageAttribution();
}

void UAskirCombatComponent::GrantDodgeInvulnerability(float Duration)
{
    if (!bDead && bDamageEnabled && GetWorld() && FMath::IsFinite(Duration) && Duration > 0.f)
        DodgeInvulnerableUntil = FMath::Max(DodgeInvulnerableUntil, GetWorld()->GetTimeSeconds() + FMath::Clamp(Duration, 0.f, 2.f));
}
bool UAskirCombatComponent::IsDodgeInvulnerable() const
{
    return GetWorld() && GetWorld()->GetTimeSeconds() < DodgeInvulnerableUntil;
}

void UAskirCombatComponent::SetDamageEnabled(bool bEnabled)
{
    bDamageEnabled=bEnabled;
    if(!bEnabled)
    {
        DodgeInvulnerableUntil=-1.f;
        ClearDamageAttribution();
    }
}

AActor* UAskirCombatComponent::ResolveDamageRecipient(AActor* DamageSource)
{
    // Resolve now: a projectile can be destroyed before the target's death delegate.
    // Ownership cycles and malformed long chains must not hang damage application.
    TSet<AActor*> Visited;
    AActor* Current=DamageSource;
    for(int32 Depth=0; Depth<16 && IsValid(Current) && !Visited.Contains(Current); ++Depth)
    {
        Visited.Add(Current);
        if(APawn* Pawn=Cast<APawn>(Current))return Pawn;
        if(AController* Controller=Cast<AController>(Current))
            if(IsValid(Controller->GetPawn()))return Controller->GetPawn();
        if(APawn* Instigator=Current->GetInstigator())
            if(IsValid(Instigator))return Instigator;
        AActor* Owner=Current->GetOwner();
        if(!IsValid(Owner) || Visited.Contains(Owner))return Current;
        Current=Owner;
    }
    return IsValid(Current) ? Current : nullptr;
}

AActor* UAskirCombatComponent::GetLastDamageCauser() const
{
    const UWorld* World=GetWorld();
    if(!World || LastDamageTimeSeconds<0.0 || !FMath::IsFinite(KillCreditWindowSeconds) || KillCreditWindowSeconds<=0.f) return nullptr;
    const double Elapsed=World->GetTimeSeconds()-LastDamageTimeSeconds;
    if(Elapsed<0.0 || Elapsed>static_cast<double>(KillCreditWindowSeconds)) return nullptr;
    return LastDamageCauser.Get();
}
bool UAskirCombatComponent::IsKillAttributedTo(AActor* Recipient) const
{
    return bDead && IsValid(Recipient) && GetLastDamageCauser()==Recipient;
}
void UAskirCombatComponent::ClearDamageAttribution()
{
    LastDamageCauser.Reset();
    LastDamageTimeSeconds=-1.0;
}
