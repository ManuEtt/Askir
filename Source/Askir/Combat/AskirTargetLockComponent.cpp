#include "Combat/AskirTargetLockComponent.h"
#include "Combat/AskirTargetingRules.h"
#include "Combat/AskirCombatTargetPolicy.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

namespace
{
AskirTargetingRules::Vector Portable(const FVector& V) { return {V.X,V.Y,V.Z}; }
bool ValidVector(const FVector& V) { return AskirTargetingRules::Finite(Portable(V)); }
double RadiusOrDefault(float Value,double Default)
{
    return FMath::IsFinite(Value)&&Value>0.f?FMath::Clamp(static_cast<double>(Value),100.,6000.):Default;
}
}

UAskirTargetLockComponent::UAskirTargetLockComponent()
{
    PrimaryComponentTick.bCanEverTick=true;
    PrimaryComponentTick.bStartWithTickEnabled=false;
    PrimaryComponentTick.TickGroup=TG_PrePhysics;
}
AAskirCharacter* UAskirTargetLockComponent::GetReadyOwner() const
{
    AAskirCharacter* Character=Cast<AAskirCharacter>(GetOwner());
    return IsValid(Character)&&Character->IsPlayerControlled()&&Character->CanUseGameplayInput()&&
        IsValid(Character->GetController())?Character:nullptr;
}
FVector UAskirTargetLockComponent::TargetPoint(const AActor* Target)
{
    return Target?Target->GetActorLocation()+FVector(0.,0.,40.):FVector::ZeroVector;
}
bool UAskirTargetLockComponent::IsEligible(const AActor* Target,double Radius) const
{
    const AAskirCharacter* Character=GetReadyOwner();
    if(!Character||!IsValid(Target)||Target->GetWorld()!=Character->GetWorld()||Target->IsHidden()||
        !AskirCombatTargetPolicy::CanDamage(Character,Target))return false;
    AskirTargetingRules::Vector Direction;double Distance=0.;
    return AskirTargetingRules::Direction(Portable(Character->GetActorLocation()),Portable(Target->GetActorLocation()),Direction,Distance)&&
        AskirTargetingRules::InRange(Distance,Radius);
}
bool UAskirTargetLockComponent::HasLineOfSight(const AActor* Target) const
{
    const AAskirCharacter* Character=GetReadyOwner();
    if(!Character||!IsValid(Target)||!GetWorld())return false;
    FVector Eye;FRotator Rotation;Character->GetActorEyesViewPoint(Eye,Rotation);
    const FVector Aim=TargetPoint(Target);
    if(!ValidVector(Eye)||!ValidVector(Aim))return false;
    FHitResult Hit;FCollisionQueryParams Query(SCENE_QUERY_STAT(AskirTargetLockSight),false,Character);
    return !GetWorld()->LineTraceSingleByChannel(Hit,Eye,Aim,ECC_Visibility,Query)||Hit.GetActor()==Target;
}
AActor* UAskirTargetLockComponent::GetLockedTarget() const
{
    AActor* Target=LockedTarget.Get();
    return IsEligible(Target,RadiusOrDefault(RetentionRadius,1800.))?Target:nullptr;
}
bool UAskirTargetLockComponent::IsTargetVisible() const
{
    const AActor* Target=GetLockedTarget();return Target&&HasLineOfSight(Target);
}
bool UAskirTargetLockComponent::GetAimDirection(const FVector& Origin,FVector& OutDirection) const
{
    const AActor* Target=GetLockedTarget();
    if(!Target||!HasLineOfSight(Target)||!GetWorld()||!ValidVector(Origin))return false;
    // A separate muzzle-to-target trace prevents an eye-visible target being shot
    // through nearby cover. Melee still performs its own collision sweep.
    const FVector Aim=TargetPoint(Target);
    FHitResult Hit;FCollisionQueryParams Query(SCENE_QUERY_STAT(AskirTargetLockAim),false,GetOwner());
    if(GetWorld()->LineTraceSingleByChannel(Hit,Origin,Aim,ECC_Visibility,Query)&&Hit.GetActor()!=Target)return false;
    AskirTargetingRules::Vector Direction;double Distance=0.;
    if(!AskirTargetingRules::Direction(Portable(Origin),Portable(Aim),Direction,Distance))return false;
    OutDirection=FVector(Direction.X,Direction.Y,Direction.Z);return true;
}
AActor* UAskirTargetLockComponent::ChooseTarget(int32 Direction) const
{
    const AAskirCharacter* Character=GetReadyOwner();
    if(!Character||!GetWorld())return nullptr;
    const double Radius=RadiusOrDefault(AcquisitionRadius,1400.);
    const double HalfAngle=FMath::IsFinite(AcquisitionHalfAngleDegrees)?FMath::Clamp(static_cast<double>(AcquisitionHalfAngleDegrees),1.,89.):75.;
    const double MinimumAlignment=FMath::Cos(FMath::DegreesToRadians(HalfAngle));
    FVector Forward=Character->GetController()->GetControlRotation().Vector();
    if(!ValidVector(Forward))return nullptr;
    if(FVector2D(Forward.X,Forward.Y).SizeSquared()<1.e-8)Forward=Character->GetActorForwardVector().GetSafeNormal2D();
    const FVector Origin=Character->GetActorLocation();
    AActor* Current=GetLockedTarget();
    AskirTargetingRules::Metrics CurrentMetrics;
    bool bCycling=Direction!=0&&Current;
    if(bCycling&&!AskirTargetingRules::Evaluate(Portable(Origin),Portable(Forward),Portable(Current->GetActorLocation()),
        RadiusOrDefault(RetentionRadius,1800.),-1.,CurrentMetrics))bCycling=false;
    AActor* Best=nullptr;AskirTargetingRules::Metrics BestMetrics;double BestOffset=0.;
    for(TActorIterator<AAskirNPC> It(GetWorld());It;++It)
    {
        AActor* Candidate=*It;
        if((bCycling&&Candidate==Current)||!IsEligible(Candidate,Radius)||!HasLineOfSight(Candidate))continue;
        AskirTargetingRules::Metrics Metrics;
        if(!AskirTargetingRules::Evaluate(Portable(Origin),Portable(Forward),Portable(Candidate->GetActorLocation()),Radius,MinimumAlignment,Metrics))continue;
        const double Offset=bCycling?AskirTargetingRules::CycleOffset(Metrics.Yaw,CurrentMetrics.Yaw,Direction):0.;
        const int Rank=Best?AskirTargetingRules::CompareRank(Metrics,BestMetrics):-1;
        const bool bPrefer=!Best||(bCycling&&Offset<BestOffset)||
            ((!bCycling||Offset==BestOffset)&&(Rank<0||(Rank==0&&Candidate->GetPathName()<Best->GetPathName())));
        if(bPrefer){Best=Candidate;BestMetrics=Metrics;BestOffset=Offset;}
    }
    return Best;
}
bool UAskirTargetLockComponent::SetLock(AActor* Target)
{
    AAskirCharacter* Character=GetReadyOwner();
    if(!Character||!GetWorld()||!IsEligible(Target,RadiusOrDefault(AcquisitionRadius,1400.))||!HasLineOfSight(Target))return false;
    const double Now=GetWorld()->GetTimeSeconds();
    if(!FMath::IsFinite(Now)||Now<0.)return false;
    UCharacterMovementComponent* Movement=Character->GetCharacterMovement();if(!Movement)return false;
    if(!bRotationFlagsSaved)
    {
        bOriginalOrientToMovement=Movement->bOrientRotationToMovement;
        bOriginalControllerDesiredRotation=Movement->bUseControllerDesiredRotation;
        bOriginalControllerYaw=Character->bUseControllerRotationYaw;
        bRotationFlagsSaved=true;
    }
    Movement->bOrientRotationToMovement=false;
    Movement->bUseControllerDesiredRotation=false;
    Character->bUseControllerRotationYaw=false;
    LockedTarget=Target;LastVisibleTime=Now;SetComponentTickEnabled(true);return true;
}
bool UAskirTargetLockComponent::ToggleLock()
{
    if(LockedTarget.IsValid()||bRotationFlagsSaved){ClearLock();return false;}
    return SetLock(ChooseTarget(0));
}
bool UAskirTargetLockComponent::CycleTarget(int32 Direction)
{
    if(Direction==0||!GetReadyOwner())return false;
    if(!GetLockedTarget()){ClearLock();return SetLock(ChooseTarget(0));}
    if(AActor* Candidate=ChooseTarget(Direction))return SetLock(Candidate);
    return false; // With a single candidate, retain the current lock.
}
void UAskirTargetLockComponent::ClearLock()
{
    if(bRotationFlagsSaved)
    {
        if(AAskirCharacter* Character=Cast<AAskirCharacter>(GetOwner()))
        {
            if(UCharacterMovementComponent* Movement=Character->GetCharacterMovement())
            {
                Movement->bOrientRotationToMovement=bOriginalOrientToMovement;
                Movement->bUseControllerDesiredRotation=bOriginalControllerDesiredRotation;
            }
            Character->bUseControllerRotationYaw=bOriginalControllerYaw;
        }
    }
    bRotationFlagsSaved=false;LockedTarget.Reset();LastVisibleTime=-1.;SetComponentTickEnabled(false);
}
void UAskirTargetLockComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
    AAskirCharacter* Character=GetReadyOwner();AActor* Target=GetLockedTarget();
    if(!Character||!Target||!GetWorld()||!FMath::IsFinite(DeltaTime)||DeltaTime<0.f){ClearLock();return;}
    const double Now=GetWorld()->GetTimeSeconds();
    if(!FMath::IsFinite(Now)||Now<0.){ClearLock();return;}
    if(HasLineOfSight(Target))LastVisibleTime=Now;
    else
    {
        const double Grace=FMath::IsFinite(OcclusionGraceSeconds)?FMath::Clamp(static_cast<double>(OcclusionGraceSeconds),0.,1.):.35;
        if(AskirTargetingRules::OcclusionExpired(Now,LastVisibleTime,Grace)){ClearLock();return;}
    }
    FVector Eye;FRotator EyeRotation;Character->GetActorEyesViewPoint(Eye,EyeRotation);
    const FVector Aim=TargetPoint(Target)-Eye;
    if(!ValidVector(Aim)||Aim.IsNearlyZero()){ClearLock();return;}
    FRotator Wanted=Aim.Rotation();Wanted.Pitch=FMath::Clamp(FRotator::NormalizeAxis(Wanted.Pitch),-65.,65.);Wanted.Roll=0.;
    const float Speed=FMath::IsFinite(CameraInterpolationSpeed)?FMath::Clamp(CameraInterpolationSpeed,1.f,30.f):10.f;
    AController* Controller=Character->GetController();
    if(Controller->GetControlRotation().ContainsNaN()){ClearLock();return;}
    Controller->SetControlRotation(FMath::RInterpTo(Controller->GetControlRotation(),Wanted,DeltaTime,Speed));
    const FVector Facing=(Target->GetActorLocation()-Character->GetActorLocation()).GetSafeNormal2D();
    if(!Facing.IsNearlyZero())Character->SetActorRotation(FMath::RInterpTo(Character->GetActorRotation(),FRotator(0.,Facing.Rotation().Yaw,0.),DeltaTime,12.f));
}
void UAskirTargetLockComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    ClearLock();Super::EndPlay(Reason);
}
