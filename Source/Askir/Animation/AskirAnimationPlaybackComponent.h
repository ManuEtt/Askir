#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AskirAnimationStateComponent.h"
#include "AskirAnimationPlaybackComponent.generated.h"

class UAnimSequence;
class USkeletalMesh;

/** Plays imported V78 skeletal animation sequences without requiring an Animation Blueprint. */
UCLASS(ClassGroup=(Askir), BlueprintType, meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirAnimationPlaybackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAskirAnimationPlaybackComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Animation|Assets")
    TSoftObjectPtr<USkeletalMesh> CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Animation|Assets")
    TMap<FName, TSoftObjectPtr<UAnimSequence>> AnimationClips;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Animation")
    bool bInstallOnBeginPlay = true;

    /** Existing designer-assigned skeletal meshes are preserved unless explicitly enabled. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Animation")
    bool bReplaceExistingMesh = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Animation", meta=(ClampMin="0.0"))
    float RunSpeedThreshold = 300.f;

    /** Generated meshes face +X, stand at Z=0 and are expressed in Unreal centimeters. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Animation")
    FRotator MeshRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Askir|Animation")
    FVector MeshScale = FVector::OneVector;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Askir|Animation")
    bool bVisualInstalled = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Askir|Animation")
    FName CurrentClip;

    /** Configure Hero, Guard or Raider before TryInstallVisual. Unsupported roles return false. */
    UFUNCTION(BlueprintCallable, Category="Askir|Animation")
    bool ConfigureCharacterVisual(FName Role);

    /** Requires the mesh and all twelve sequences, with the exact same skeleton. */
    UFUNCTION(BlueprintCallable, Category="Askir|Animation")
    bool TryInstallVisual();

    /** Call after AnimationState::ResetAfterLoad; dead saves immediately show the terminal pose. */
    UFUNCTION(BlueprintCallable, Category="Askir|Animation")
    void RestorePlaybackAfterLoad();

    UFUNCTION(BlueprintPure, Category="Askir|Animation")
    static FName SelectClip(EAskirActionPose Action, bool bDead, bool bBlocking,
        bool bInAir, float GroundSpeed, bool bSprinting, float RunThreshold);

    UFUNCTION(BlueprintPure, Category="Askir|Animation")
    static bool IsLoopingClip(FName Clip);

private:
    UPROPERTY(Transient) TObjectPtr<USkeletalMesh> InstalledMesh;
    UPROPERTY(Transient) TMap<FName, TObjectPtr<UAnimSequence>> LoadedClips;
    TWeakObjectPtr<UAskirAnimationStateComponent> State;
    int32 LastActionSequence = INDEX_NONE;
    float PlaybackElapsed = 0.f;
    bool bFinalFrameHeld = false;
    void UpdatePlayback(float DeltaTime);
};
