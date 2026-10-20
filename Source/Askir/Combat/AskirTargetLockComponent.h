#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirTargetLockComponent.generated.h"

class AAskirCharacter;

/** Local third-person target lock. Target references are transient and never saved. */
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirTargetLockComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UAskirTargetLockComponent();
    virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    UFUNCTION(BlueprintCallable,Category="Askir|Combat|Target") bool ToggleLock();
    UFUNCTION(BlueprintCallable,Category="Askir|Combat|Target") bool CycleTarget(int32 Direction=1);
    UFUNCTION(BlueprintCallable,Category="Askir|Combat|Target") void ClearLock();
    UFUNCTION(BlueprintPure,Category="Askir|Combat|Target") AActor* GetLockedTarget() const;
    UFUNCTION(BlueprintPure,Category="Askir|Combat|Target") bool IsLocked() const { return GetLockedTarget()!=nullptr; }
    UFUNCTION(BlueprintPure,Category="Askir|Combat|Target") bool IsTargetVisible() const;
    /** Fresh direct line of sight is mandatory; this never adds projectile homing. */
    UFUNCTION(BlueprintCallable,Category="Askir|Combat|Target") bool GetAimDirection(const FVector& Origin,FVector& OutDirection) const;

    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Combat|Target",meta=(ClampMin="100.0",ClampMax="5000.0")) float AcquisitionRadius=1400.f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Combat|Target",meta=(ClampMin="100.0",ClampMax="6000.0")) float RetentionRadius=1800.f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Combat|Target",meta=(ClampMin="1.0",ClampMax="89.0")) float AcquisitionHalfAngleDegrees=75.f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Combat|Target",meta=(ClampMin="0.0",ClampMax="1.0")) float OcclusionGraceSeconds=.35f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Combat|Target",meta=(ClampMin="1.0",ClampMax="30.0")) float CameraInterpolationSpeed=10.f;
private:
    UPROPERTY(Transient) TWeakObjectPtr<AActor> LockedTarget;
    bool bRotationFlagsSaved=false;
    bool bOriginalOrientToMovement=true;
    bool bOriginalControllerDesiredRotation=false;
    bool bOriginalControllerYaw=false;
    double LastVisibleTime=-1.;
    AAskirCharacter* GetReadyOwner() const;
    bool IsEligible(const AActor* Target,double Radius) const;
    bool HasLineOfSight(const AActor* Target) const;
    bool SetLock(AActor* Target);
    AActor* ChooseTarget(int32 Direction) const;
    static FVector TargetPoint(const AActor* Target);
};
