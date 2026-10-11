#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AskirAttackImpactNotify.generated.h"
UCLASS(meta=(DisplayName="Askir Attack Impact"))
class ASKIR_API UAskirAttackImpactNotify : public UAnimNotify
{
    GENERATED_BODY()
public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;
};
