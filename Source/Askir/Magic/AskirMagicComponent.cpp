#include "Magic/AskirMagicComponent.h"
#include "Magic/AskirSpellCatalog.h"
#include "Magic/AskirSpellProjectile.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Combat/AskirCombatComponent.h"
#include "Combat/AskirCombatTargetPolicy.h"
#include "Combat/AskirTargetLockComponent.h"
#include "Animation/AskirAnimationStateComponent.h"
#include "Audio/AskirAudioLibrary.h"
#include "Game/AskirPlayerController.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

using namespace AskirMagicRules;
UAskirMagicComponent::UAskirMagicComponent(){PrimaryComponentTick.bCanEverTick=true;}
double UAskirMagicComponent::Now()const{return GetWorld()?GetWorld()->GetTimeSeconds():0.0;}
void UAskirMagicComponent::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta,Type,Tick);
    const auto* Combat=GetOwner()?GetOwner()->FindComponentByClass<UAskirCombatComponent>():nullptr;
    if(Combat&&!Combat->IsDead()&&Combat->IsDamageEnabled())Mana=static_cast<float>(Regenerate(Mana,MaxMana,ManaRegenPerSecond,Delta));
    if(Now()>=ShieldUntil)ShieldCapacity=0.f;
}
bool UAskirMagicComponent::IsCasting()const{return Now()<CastingUntil;}
void UAskirMagicComponent::CancelCasting(){CastingUntil=-1.0;}
FName UAskirMagicComponent::GetSelectedSpellID()const{return FName(UTF8_TO_TCHAR(Catalog[SelectedIndex].ID));}
FText UAskirMagicComponent::GetSelectedSpellName()const{return FText::FromString(UTF8_TO_TCHAR(Catalog[SelectedIndex].Name));}
float UAskirMagicComponent::GetSelectedCooldownRemaining()const
{
    const double* Ready=ReadyAt.Find(GetSelectedSpellID());return Ready?FMath::Max(0.f,static_cast<float>(*Ready-Now())):0.f;
}
float UAskirMagicComponent::GetShield()const{return Now()<ShieldUntil?ShieldCapacity:0.f;}
bool UAskirMagicComponent::SelectSpell(FName ID)
{
    if(bCastTransaction)return false;
    for(int32 I=0;I<SpellCount;++I)if(ID==FName(UTF8_TO_TCHAR(Catalog[I].ID))){SelectedIndex=I;return true;}
    return false;
}
void UAskirMagicComponent::CycleSpell(int32 Direction)
{
    if(!bCastTransaction&&Direction!=0)SelectedIndex=(SelectedIndex+(Direction>0?1:SpellCount-1))%SpellCount;
}
bool UAskirMagicComponent::IsValidTarget(AActor* Caster,AActor* Target)
{
    return AskirCombatTargetPolicy::CanDamage(Caster,Target);
}
float UAskirMagicComponent::ApplySpellHit(AActor* Caster,AActor* Target,Element School,float Power,const FVector& Location,const FVector& TravelDirection)
{
    if(!IsValidTarget(Caster,Target)||!FMath::IsFinite(Power)||Power<=0||TravelDirection.ContainsNaN())return 0.f;
    const EAskirDamageType Types[]={EAskirDamageType::Fire,EAskirDamageType::Frost,EAskirDamageType::Lightning,EAskirDamageType::Physical,EAskirDamageType::Magic,EAskirDamageType::Magic};
    const int32 Index=static_cast<int32>(School);if(Index<0||Index>=6)return 0.f;
    FAskirDamageInfo Info;Info.Damage=Power;Info.DamageType=Types[Index];Info.DamageCauser=Caster;Info.HitLocation=Location;
    Info.HitDirection=TravelDirection.IsNearlyZero()?(Target->GetActorLocation()-Caster->GetActorLocation()).GetSafeNormal():TravelDirection.GetSafeNormal();
    auto* Combat=Target->FindComponentByClass<UAskirCombatComponent>();
    const float Before=Combat->GetHealth();
    if(auto* Character=Cast<AAskirCharacter>(Target))Character->ReceiveAskirDamage(Info);else Combat->ApplyDamage(Info);
    const float Applied=FMath::Max(0.f,Before-Combat->GetHealth());
    if(Applied>0.f)UAskirAudioLibrary::PlayWorldCue(Target,TEXT("SpellImpact"));
    return Applied;
}
bool UAskirMagicComponent::CastSelected()
{
    if(bCastTransaction||!GetWorld())return false;
    auto* Character=Cast<AAskirCharacter>(GetOwner());auto* Pawn=Cast<APawn>(GetOwner());
    const Spell& S=Catalog[SelectedIndex];
    LastFailure=CheckCast(S,Mana,GetSelectedCooldownRemaining(),Character&&!Character->IsDead(),Character&&Character->CombatComponent->IsDamageEnabled(),Character&&!Character->CanUseGameplayInput(),IsCasting()||(Character&&(Character->HasPendingAttack()||Character->IsBlocking())));
    if(LastFailure!=Failure::None||!Pawn)return false;
    TGuardValue<bool> Guard(bCastTransaction,true);
    const FVector CastOrigin=Pawn->GetActorLocation()+FVector(0,0,40);
    FVector Direction=Pawn->GetController()?Pawn->GetController()->GetControlRotation().Vector():Pawn->GetActorForwardVector();
    FVector LockedDirection;
    if(Character->TargetLockComponent&&Character->TargetLockComponent->GetAimDirection(CastOrigin,LockedDirection))Direction=LockedDirection;
    if(Direction.ContainsNaN()||!Direction.Normalize()){LastFailure=Failure::Unavailable;return false;}
    AAskirSpellProjectile* Projectile=nullptr;
    if(S.Kind==Effect::Projectile)
    {
        FVector Location=CastOrigin+Direction*70.f;
        // Check the whole launch segment too: AlwaysSpawn must not place a spell
        // on the far side of nearby geometry before its first movement sweep.
        FHitResult LaunchHit;FCollisionQueryParams LaunchQuery(SCENE_QUERY_STAT(AskirSpellLaunch),false,Pawn);
        if(GetWorld()->SweepSingleByChannel(LaunchHit,CastOrigin,Location,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(12.f),LaunchQuery))
        {
            if(!IsValidTarget(Pawn,LaunchHit.GetActor())){LastFailure=Failure::Unavailable;return false;}
            Location=CastOrigin;
        }
        FActorSpawnParameters Params;Params.Owner=Pawn;Params.Instigator=Pawn;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Projectile=GetWorld()->SpawnActor<AAskirSpellProjectile>(Location,Direction.Rotation(),Params);
        if(!Projectile){LastFailure=Failure::Unavailable;return false;}
        Projectile->InitializeSpell(Pawn,S.School,static_cast<float>(S.Power),Direction);
    }
    // Resource receipts are installed before damage, UI or XP delegates may run.
    Mana-=static_cast<float>(S.ManaCost);ReadyAt.Add(GetSelectedSpellID(),Now()+S.Cooldown);CastingUntil=Now()+.65;
    if(Character->AnimationState)Character->AnimationState->BeginCast();
    if(S.Kind==Effect::Shield){ShieldCapacity=static_cast<float>(S.Power);ShieldUntil=Now()+10.0;}
    if(S.Kind==Effect::MeleeArc||S.Kind==Effect::Area)
    {
        const float Radius=S.Kind==Effect::Area?600.f:260.f;
        const FVector Origin=Pawn->GetActorLocation();
        TArray<FOverlapResult> Hits;FCollisionQueryParams Query(SCENE_QUERY_STAT(AskirMagicArea),false,Pawn);
        GetWorld()->OverlapMultiByObjectType(Hits,Origin,FQuat::Identity,FCollisionObjectQueryParams(ECC_Pawn),FCollisionShape::MakeSphere(Radius),Query);
        TSet<AActor*> Seen;
        for(const auto& Hit:Hits)
        {
            AActor* Target=Hit.GetActor();if(!IsValidTarget(Pawn,Target)||Seen.Contains(Target))continue;Seen.Add(Target);
            if(S.Kind==Effect::MeleeArc&&FVector::DotProduct(Direction.GetSafeNormal2D(),(Target->GetActorLocation()-Origin).GetSafeNormal2D())<.5f)continue;
            FHitResult Block;const FVector Eye=Origin+FVector(0,0,35);
            if(GetWorld()->LineTraceSingleByChannel(Block,Eye,Target->GetActorLocation()+FVector(0,0,35),ECC_Visibility,Query)&&Block.GetActor()!=Target)continue;
            ApplySpellHit(Pawn,Target,S.School,static_cast<float>(S.Power),Target->GetActorLocation());
        }
        if(S.School==Element::Light&&S.Kind==Effect::Area)Character->Heal(static_cast<float>(S.Power*.8));
    }
    UAskirAudioLibrary::PlayCue(this,TEXT("SpellCast"));OnSpellCast.Broadcast(GetSelectedSpellID());return true;
}
float UAskirMagicComponent::AbsorbIncomingDamage(float Damage,bool bBypass)
{
    double Capacity=ShieldCapacity;const double Result=ConsumeShield(Damage,Capacity,ShieldUntil-Now(),bBypass);ShieldCapacity=static_cast<float>(Capacity);return static_cast<float>(Result);
}
FAskirMagicSnapshot UAskirMagicComponent::CaptureSnapshot()const
{
    FAskirMagicSnapshot S;S.Mana=Mana;S.SelectedSpellID=GetSelectedSpellID();
    for(const auto& Pair:ReadyAt)if(Pair.Value>Now())S.CooldownsRemaining.Add(Pair.Key,static_cast<float>(Pair.Value-Now()));
    S.ShieldCapacity=GetShield();S.ShieldSecondsRemaining=S.ShieldCapacity>0?static_cast<float>(FMath::Max(0.0,ShieldUntil-Now())):0.f;return S;
}
bool UAskirMagicComponent::CanRestoreSnapshot(const FAskirMagicSnapshot& S)const
{
    return !bCastTransaction&&ValidateSnapshotData(S,MaxMana);
}
bool UAskirMagicComponent::ValidateSnapshotData(const FAskirMagicSnapshot& S,float ManaCapacity)
{
    if(!ValidMana(S.Mana,ManaCapacity)||S.CooldownsRemaining.Num()>SpellCount||!ValidDuration(S.ShieldSecondsRemaining,10)||!FMath::IsFinite(S.ShieldCapacity)||S.ShieldCapacity<0||S.ShieldCapacity>30.001f||(S.ShieldCapacity>0)!=(S.ShieldSecondsRemaining>0))return false;
    bool bKnown=false;for(const Spell& Spell:Catalog)if(S.SelectedSpellID==FName(UTF8_TO_TCHAR(Spell.ID)))bKnown=true;
    if(!bKnown)return false;
    for(const auto& Pair:S.CooldownsRemaining)
    {
        const Spell* Found=nullptr;for(const Spell& Spell:Catalog)if(Pair.Key==FName(UTF8_TO_TCHAR(Spell.ID)))Found=&Spell;
        if(!Found||!ValidDuration(Pair.Value,Found->Cooldown))return false;
    }
    return true;
}
bool UAskirMagicComponent::RestoreSnapshot(const FAskirMagicSnapshot& S)
{
    if(bCastTransaction||!CanRestoreSnapshot(S))return false;
    Mana=S.Mana;SelectSpell(S.SelectedSpellID);ReadyAt.Reset();for(const auto& Pair:S.CooldownsRemaining)ReadyAt.Add(Pair.Key,Now()+Pair.Value);
    ShieldCapacity=S.ShieldCapacity;ShieldUntil=Now()+S.ShieldSecondsRemaining;CancelCasting();return true;
}
FText UAskirMagicComponent::GetLastFailureText()const
{
    switch(LastFailure)
    {
    case Failure::Mana:return NSLOCTEXT("AskirMagic","Mana","Nicht genug Mana.");
    case Failure::Cooldown:return NSLOCTEXT("AskirMagic","Cooldown","Der Zauber ist noch nicht bereit.");
    case Failure::Busy:return NSLOCTEXT("AskirMagic","Busy","Beende zuerst deine aktuelle Aktion.");
    case Failure::None:return FText::GetEmpty();
    default:return NSLOCTEXT("AskirMagic","Unavailable","Du kannst derzeit nicht zaubern.");
    }
}
