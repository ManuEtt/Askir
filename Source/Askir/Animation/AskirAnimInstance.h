#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AskirAnimationStateComponent.h"
#include "AskirAnimInstance.generated.h"

// Native animation bridge. Assign this AnimInstance (or a BP subclass) to a rigged SkeletalMesh.
UCLASS(Blueprintable, BlueprintType)
class ASKIR_API UAskirAnimInstance : public UAnimInstance
{
    GENERATED_BODY()
public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadOnly, Category="Askir|Locomotion") float Speed=0.f;
    UPROPERTY(BlueprintReadOnly, Category="Askir|Locomotion") float Direction=0.f;
    UPROPERTY(BlueprintReadOnly, Category="Askir|Locomotion") bool bMoving=false;
    UPROPERTY(BlueprintReadOnly, Category="Askir|Locomotion") bool bFalling=false;
    UPROPERTY(BlueprintReadOnly, Category="Askir|Locomotion") bool bSprinting=false;
    UPROPERTY(BlueprintReadOnly, Category="Askir|Combat") bool bBlocking=false;
    UPROPERTY(BlueprintReadOnly, Category="Askir|Combat") bool bDead=false;
    UPROPERTY(BlueprintReadOnly, Category="Askir|Combat") EAskirActionPose CurrentAction=EAskirActionPose::None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Askir|Montages") TObjectPtr<UAnimMontage> LightAttackMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Askir|Montages") TObjectPtr<UAnimMontage> HeavyAttackMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Askir|Montages") TObjectPtr<UAnimMontage> DodgeMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Askir|Montages") TObjectPtr<UAnimMontage> BlockMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Askir|Montages") TObjectPtr<UAnimMontage> HitReactMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Askir|Montages") TObjectPtr<UAnimMontage> DeathMontage;

    UFUNCTION(BlueprintCallable, Category="Askir|Animation") void ResetActionAfterMontage();
private:
    UPROPERTY(Transient) TObjectPtr<UAskirAnimationStateComponent> State;
    int32 LastSequence=INDEX_NONE;
    void PlayAction(EAskirActionPose Action);
};
