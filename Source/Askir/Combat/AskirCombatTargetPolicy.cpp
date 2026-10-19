#include "Combat/AskirCombatTargetPolicy.h"
#include "Combat/AskirCombatTargetRules.h"
#include "Combat/AskirCombatComponent.h"
#include "Combat/AskirDamageTypes.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Frontend/AskirFrontendSubsystem.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Travel/AskirWorldTravelSubsystem.h"

namespace
{
using AskirCombatTargetRules::Vector;
Vector ToRules(const FVector& Value){return {Value.X,Value.Y,Value.Z};}
bool IsLiveActor(const AActor* Actor)
{
    return IsValid(Actor) && !Actor->IsActorBeingDestroyed() && IsValid(Actor->GetWorld()) && !Actor->GetWorld()->bIsTearingDown;
}
AskirCombatTargetRules::Participant Describe(const AActor* Actor)
{
    AskirCombatTargetRules::Participant Result;
    Result.Valid=IsLiveActor(Actor);
    if(!Result.Valid)return Result;
    const auto* Combat=Actor->FindComponentByClass<UAskirCombatComponent>();
    Result.Dead=!IsValid(Combat) || Combat->IsDead();
    Result.DamageEnabled=IsValid(Combat) && Combat->IsDamageEnabled();
    if(const auto* Pawn=Cast<APawn>(Actor))Result.PlayerControlled=Pawn->IsPlayerControlled();
    if(const auto* NPC=Cast<AAskirNPC>(Actor))
    {
        Result.HostileNPC=NPC->bHostile;
        Result.EncounterActive=NPC->IsEncounterActive();
    }
    if(const auto* Character=Cast<AAskirCharacter>(Actor))Result.TravelPending=Character->IsWorldTravelPending();
    else if(const auto* Instance=Actor->GetWorld()->GetGameInstance())
    {
        if(const auto* Frontend=Instance->GetSubsystem<UAskirFrontendSubsystem>())Result.TravelPending=Frontend->IsFreshStartPending();
        if(const auto* Travel=Instance->GetSubsystem<UAskirWorldTravelSubsystem>())Result.TravelPending|=Travel->IsGameplayBlocked();
    }
    return Result;
}

bool FindSourceDirection(const AActor* Defender,const AActor* Causer,FVector& Direction)
{
    if(Causer==Defender)return false;
    TSet<const AActor*> Visited;
    const AActor* Current=Causer;
    for(int32 Depth=0;Depth<16 && IsLiveActor(Current) && !Visited.Contains(Current);++Depth)
    {
        if(Current->GetWorld()!=Defender->GetWorld())return false;
        Visited.Add(Current);
        const FVector Location=Current->GetActorLocation();
        if(!AskirCombatTargetRules::IsFinite(ToRules(Location)))return false;
        if(Current!=Defender)
        {
            Direction=Location-Defender->GetActorLocation();
            if(AskirCombatTargetRules::HasHorizontalDirection(ToRules(Direction)))return true;
        }
        // A projectile at the impact point may have no meaningful source displacement.
        if(const auto* Controller=Cast<AController>(Current))
        {
            const APawn* Pawn=Controller->GetPawn();
            if(IsLiveActor(Pawn) && Pawn!=Defender && !Visited.Contains(Pawn)){Current=Pawn;continue;}
        }
        const APawn* Instigator=Current->GetInstigator();
        if(IsLiveActor(Instigator) && Instigator!=Defender && !Visited.Contains(Instigator)){Current=Instigator;continue;}
        Current=Current->GetOwner();
    }
    return false;
}
}

bool AskirCombatTargetPolicy::CanDamage(const AActor* Attacker,const AActor* Target)
{
    return AskirCombatTargetRules::CanDamage(Describe(Attacker),Describe(Target),
        IsLiveActor(Attacker) && IsLiveActor(Target) && Attacker->GetWorld()==Target->GetWorld(),Attacker==Target);
}

bool AskirCombatTargetPolicy::IsFrontalBlock(const AActor* Defender,const FAskirDamageInfo& Damage,float HalfAngleDegrees)
{
    if(!IsLiveActor(Defender) || Damage.DamageType==EAskirDamageType::TrueDamage
        || !AskirCombatTargetRules::IsValidHalfAngle(HalfAngleDegrees)
        || !AskirCombatTargetRules::IsFinite(ToRules(Defender->GetActorLocation()))
        || !AskirCombatTargetRules::IsFinite(ToRules(Damage.HitDirection)))return false;
    FVector SourceDirection=FVector::ZeroVector;
    const bool HasSource=!AskirCombatTargetRules::HasHorizontalDirection(ToRules(Damage.HitDirection))
        && FindSourceDirection(Defender,Damage.DamageCauser.Get(),SourceDirection);
    return AskirCombatTargetRules::IsFrontalBlock(ToRules(Defender->GetActorForwardVector()),ToRules(Damage.HitDirection),
        ToRules(SourceDirection),HasSource,HalfAngleDegrees,false);
}
