#include "Animation/AskirAttackImpactNotify.h"
#include "Characters/AskirCharacter.h"
#include "Components/SkeletalMeshComponent.h"
void UAskirAttackImpactNotify::Notify(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp,Animation,EventReference);
    if(MeshComp)
        if(AAskirCharacter* Character=Cast<AAskirCharacter>(MeshComp->GetOwner()))
            Character->ResolvePendingAttackImpact();
}
