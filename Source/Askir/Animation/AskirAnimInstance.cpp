#include "Animation/AskirAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"

void UAskirAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    const APawn* Pawn=TryGetPawnOwner();
    State=Pawn ? Pawn->FindComponentByClass<UAskirAnimationStateComponent>() : nullptr;
    LastSequence=INDEX_NONE;
}
void UAskirAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    if(!State)
    {
        const APawn* Pawn=TryGetPawnOwner();
        State=Pawn ? Pawn->FindComponentByClass<UAskirAnimationStateComponent>() : nullptr;
    }
    if(!State)return;
    Speed=State->GroundSpeed;
    Direction=State->DirectionDegrees;
    bMoving=State->bIsMoving;
    bFalling=State->bIsInAir;
    bSprinting=State->bIsSprinting;
    bBlocking=State->bIsBlocking;
    bDead=State->bIsDead;
    CurrentAction=State->Action;
    if(LastSequence!=State->ActionSequence)
    {
        LastSequence=State->ActionSequence;
        PlayAction(CurrentAction);
    }
}
void UAskirAnimInstance::PlayAction(EAskirActionPose Action)
{
    UAnimMontage* Montage=nullptr;
    switch(Action)
    {
    case EAskirActionPose::LightAttack: Montage=LightAttackMontage;break;
    case EAskirActionPose::HeavyAttack: Montage=HeavyAttackMontage;break;
    case EAskirActionPose::Dodge: Montage=DodgeMontage;break;
    case EAskirActionPose::Block: Montage=BlockMontage;break;
    case EAskirActionPose::HitReact: Montage=HitReactMontage;break;
    case EAskirActionPose::Death: Montage=DeathMontage;break;
    default: break;
    }
    if(Action==EAskirActionPose::None && BlockMontage && Montage_IsPlaying(BlockMontage))
        Montage_Stop(0.15f,BlockMontage);
    if(Montage)
    {
        if(Action==EAskirActionPose::Death)Montage_Stop(0.1f);
        Montage_Play(Montage,1.f);
    }
}
void UAskirAnimInstance::ResetActionAfterMontage()
{
    if(State && !State->bIsDead)State->FinishAction();
}
