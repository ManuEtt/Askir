#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirAnimationStateComponent.generated.h"

UENUM(BlueprintType)
enum class EAskirActionPose : uint8 { None, LightAttack, HeavyAttack, Dodge, Block, HitReact, Death, Cast };

UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirAnimationStateComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UAskirAnimationStateComponent();
    virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
    UPROPERTY(BlueprintReadOnly,Category="Askir|Animation") float GroundSpeed=0.f;
    UPROPERTY(BlueprintReadOnly,Category="Askir|Animation") float DirectionDegrees=0.f;
    UPROPERTY(BlueprintReadOnly,Category="Askir|Animation") bool bIsMoving=false;
    UPROPERTY(BlueprintReadOnly,Category="Askir|Animation") bool bIsInAir=false;
    UPROPERTY(BlueprintReadOnly,Category="Askir|Animation") bool bIsSprinting=false;
    UPROPERTY(BlueprintReadOnly,Category="Askir|Animation") bool bIsBlocking=false;
    UPROPERTY(BlueprintReadOnly,Category="Askir|Animation") bool bIsDead=false;
    UPROPERTY(BlueprintReadOnly,Category="Askir|Animation") EAskirActionPose Action=EAskirActionPose::None;
    UPROPERTY(BlueprintReadOnly,Category="Askir|Animation") int32 ActionSequence=0;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Animation") float WalkRunThreshold=550.f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Animation") float TransientActionSeconds=0.85f;
    UPROPERTY(BlueprintReadOnly,Category="Askir|Animation") float ActionElapsed=0.f;
    UFUNCTION(BlueprintCallable,Category="Askir|Animation") void RefreshMovement();
    UFUNCTION(BlueprintCallable,Category="Askir|Animation") void BeginLightAttack();
    UFUNCTION(BlueprintCallable,Category="Askir|Animation") void BeginHeavyAttack();
    UFUNCTION(BlueprintCallable,Category="Askir|Animation") void BeginCast();
    UFUNCTION(BlueprintCallable,Category="Askir|Animation") void BeginDodge();
    UFUNCTION(BlueprintCallable,Category="Askir|Animation") void SetBlocking(bool bEnabled);
    UFUNCTION(BlueprintCallable,Category="Askir|Animation") void RegisterHit();
    UFUNCTION(BlueprintCallable,Category="Askir|Animation") void RegisterDeath();
    UFUNCTION(BlueprintCallable,Category="Askir|Animation") void ResetAfterLoad(bool bDead);
    UFUNCTION(BlueprintCallable,Category="Askir|Animation") void FinishAction();
    UFUNCTION(BlueprintPure,Category="Askir|Animation") bool IsCombatAction() const;
private:
    void TriggerAction(EAskirActionPose NewAction);
};
