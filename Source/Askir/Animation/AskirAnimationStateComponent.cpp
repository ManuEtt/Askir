#include "Animation/AskirAnimationStateComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UAskirAnimationStateComponent::UAskirAnimationStateComponent(){PrimaryComponentTick.bCanEverTick=true;}
void UAskirAnimationStateComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick){Super::TickComponent(Dt,Type,Tick);RefreshMovement();if(IsCombatAction()){ActionElapsed+=Dt;if(ActionElapsed>=FMath::Max(0.05f,TransientActionSeconds))FinishAction();}}
void UAskirAnimationStateComponent::RefreshMovement()
{
    const ACharacter* Character=Cast<ACharacter>(GetOwner());
    if(!Character)return;
    const FVector Velocity=Character->GetVelocity();
    GroundSpeed=Velocity.Size2D();
    bIsMoving=GroundSpeed>3.f;
    const UCharacterMovementComponent* Move=Character->GetCharacterMovement();
    bIsInAir=Move&&Move->IsFalling();
    bIsSprinting=Move&&bIsMoving&&Move->MaxWalkSpeed>WalkRunThreshold;
    if(bIsMoving)
    {
        const FVector Local=Character->GetActorTransform().InverseTransformVectorNoScale(Velocity);
        DirectionDegrees=FMath::RadiansToDegrees(FMath::Atan2(Local.Y,Local.X));
    }
    else DirectionDegrees=0.f;
}
void UAskirAnimationStateComponent::TriggerAction(EAskirActionPose Next){if(bIsDead)return;Action=Next;ActionElapsed=0.f;++ActionSequence;}
void UAskirAnimationStateComponent::BeginLightAttack(){TriggerAction(EAskirActionPose::LightAttack);}
void UAskirAnimationStateComponent::BeginHeavyAttack(){TriggerAction(EAskirActionPose::HeavyAttack);}
void UAskirAnimationStateComponent::BeginDodge(){TriggerAction(EAskirActionPose::Dodge);}
void UAskirAnimationStateComponent::SetBlocking(bool Enabled){if(bIsDead)return;bIsBlocking=Enabled;if(Enabled)TriggerAction(EAskirActionPose::Block);else if(Action==EAskirActionPose::Block)FinishAction();}
void UAskirAnimationStateComponent::RegisterHit(){TriggerAction(EAskirActionPose::HitReact);}
void UAskirAnimationStateComponent::RegisterDeath(){bIsDead=true;bIsBlocking=false;Action=EAskirActionPose::Death;ActionElapsed=0.f;++ActionSequence;}
void UAskirAnimationStateComponent::ResetAfterLoad(bool bDead)
{
    bIsDead=bDead;
    bIsBlocking=false;
    Action=bDead?EAskirActionPose::Death:EAskirActionPose::None;
    ActionElapsed=0.f;
    ++ActionSequence;
    RefreshMovement();
}
void UAskirAnimationStateComponent::FinishAction(){if(!bIsDead){Action=bIsBlocking?EAskirActionPose::Block:EAskirActionPose::None;ActionElapsed=0.f;}}
bool UAskirAnimationStateComponent::IsCombatAction()const{return Action==EAskirActionPose::LightAttack||Action==EAskirActionPose::HeavyAttack||Action==EAskirActionPose::Dodge||Action==EAskirActionPose::HitReact;}
