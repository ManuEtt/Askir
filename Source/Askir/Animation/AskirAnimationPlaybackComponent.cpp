#include "Animation/AskirAnimationPlaybackComponent.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/PackageName.h"

namespace
{
    const FName RequiredClips[] = {
        TEXT("Idle"), TEXT("Walk"), TEXT("Run"), TEXT("Sprint"),
        TEXT("Jump"), TEXT("Dodge"), TEXT("LightAttack"), TEXT("HeavyAttack"),
        TEXT("Hit"), TEXT("Death"), TEXT("Block"), TEXT("Cast")
    };

    template<typename AssetType>
    AssetType* LoadExistingAsset(const TSoftObjectPtr<AssetType>& Reference)
    {
        if (AssetType* Loaded = Reference.Get()) return Loaded;
        const FString Package = Reference.ToSoftObjectPath().GetLongPackageName();
        // Missing imported content is an expected fallback state on a source-only checkout.
        if (!FPackageName::IsValidLongPackageName(Package) || !FPackageName::DoesPackageExist(Package))
            return nullptr;
        return Reference.LoadSynchronous();
    }
}

UAskirAnimationPlaybackComponent::UAskirAnimationPlaybackComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    ConfigureCharacterVisual(TEXT("Hero"));
}

bool UAskirAnimationPlaybackComponent::ConfigureCharacterVisual(FName Role)
{
    if (Role != TEXT("Hero") && Role != TEXT("Guard") && Role != TEXT("Raider")) return false;
    const FString Root = FString::Printf(TEXT("/Game/Askir/CharactersV78/%s/"), *Role.ToString());
    const FString MeshName = TEXT("SK_Askir") + Role.ToString();
    CharacterMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(Root + MeshName + TEXT(".") + MeshName));
    AnimationClips.Empty();
    for (const FName Clip : RequiredClips)
    {
        const FString Name = TEXT("A_") + Clip.ToString();
        AnimationClips.Add(Clip, TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(Root + Name + TEXT(".") + Name)));
    }
    return true;
}

void UAskirAnimationPlaybackComponent::BeginPlay()
{
    Super::BeginPlay();
    State = GetOwner() ? GetOwner()->FindComponentByClass<UAskirAnimationStateComponent>() : nullptr;
    if (State.IsValid()) AddTickPrerequisiteComponent(State.Get());
    if (bInstallOnBeginPlay) TryInstallVisual();
}

bool UAskirAnimationPlaybackComponent::TryInstallVisual()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character || !Character->GetMesh()) return false;
    USkeletalMeshComponent* Mesh = Character->GetMesh();
    USkeletalMesh* Candidate = LoadExistingAsset(CharacterMesh);
    if (!Candidate || !Candidate->GetSkeleton()) return false;
    if (Mesh->GetSkeletalMeshAsset() && Mesh->GetSkeletalMeshAsset() != InstalledMesh &&
        Mesh->GetSkeletalMeshAsset() != Candidate && !bReplaceExistingMesh) return false;

    TMap<FName, TObjectPtr<UAnimSequence>> Candidates;
    for (const FName Clip : RequiredClips)
    {
        const TSoftObjectPtr<UAnimSequence>* Reference = AnimationClips.Find(Clip);
        UAnimSequence* Sequence = Reference ? LoadExistingAsset(*Reference) : nullptr;
        if (!Sequence || Sequence->GetSkeleton() != Candidate->GetSkeleton() ||
            !FMath::IsFinite(Sequence->GetPlayLength()) || Sequence->GetPlayLength() <= 0.f) return false;
        Candidates.Add(Clip, Sequence);
    }
    // Asset validation is complete before changing any visible component.
    LoadedClips = MoveTemp(Candidates);
    InstalledMesh = Candidate;
    Mesh->SetSkeletalMesh(Candidate);
    Mesh->SetRelativeLocation(FVector(0.f, 0.f, -Character->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()));
    Mesh->SetRelativeRotation(MeshRotation);
    Mesh->SetRelativeScale3D(MeshScale);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->SetVisibility(true);
    Mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    Mesh->AddTickPrerequisiteComponent(this);
    State = Character->FindComponentByClass<UAskirAnimationStateComponent>();
    if (State.IsValid()) AddTickPrerequisiteComponent(State.Get());
    bVisualInstalled = true;
    CurrentClip = NAME_None;
    LastActionSequence = INDEX_NONE;
    UpdatePlayback(0.f);
    return true;
}

FName UAskirAnimationPlaybackComponent::SelectClip(EAskirActionPose Action, bool bDead,
    bool bBlocking, bool bInAir, float Speed, bool bSprinting, float RunThreshold)
{
    if (bDead || Action == EAskirActionPose::Death) return TEXT("Death");
    switch (Action)
    {
        case EAskirActionPose::LightAttack: return TEXT("LightAttack");
        case EAskirActionPose::HeavyAttack: return TEXT("HeavyAttack");
        case EAskirActionPose::Dodge: return TEXT("Dodge");
        case EAskirActionPose::HitReact: return TEXT("Hit");
        case EAskirActionPose::Cast: return TEXT("Cast");
        default: break;
    }
    if (bBlocking || Action == EAskirActionPose::Block) return TEXT("Block");
    if (bInAir) return TEXT("Jump");
    if (!FMath::IsFinite(Speed) || Speed <= 3.f) return TEXT("Idle");
    if (bSprinting) return TEXT("Sprint");
    return Speed >= FMath::Max(3.f, RunThreshold) ? FName(TEXT("Run")) : FName(TEXT("Walk"));
}

bool UAskirAnimationPlaybackComponent::IsLoopingClip(FName Clip)
{
    return Clip == TEXT("Idle") || Clip == TEXT("Walk") || Clip == TEXT("Run") ||
        Clip == TEXT("Sprint") || Clip == TEXT("Block");
}

void UAskirAnimationPlaybackComponent::RestorePlaybackAfterLoad()
{
    if (!bVisualInstalled) return;
    CurrentClip = NAME_None;
    LastActionSequence = INDEX_NONE;
    UpdatePlayback(0.f);
    if (CurrentClip != TEXT("Death")) return;
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    const TObjectPtr<UAnimSequence>* Reference = LoadedClips.Find(TEXT("Death"));
    UAnimSequence* Sequence = Reference ? Reference->Get() : nullptr;
    USkeletalMeshComponent* Mesh = Character ? Character->GetMesh() : nullptr;
    if (Mesh && Sequence)
    {
        if (UAnimSingleNodeInstance* Node = Mesh->GetSingleNodeInstance())
        {
            Node->SetPosition(Sequence->GetPlayLength(), false);
            Node->SetPlaying(false);
            PlaybackElapsed = Sequence->GetPlayLength();
            bFinalFrameHeld = true;
        }
    }
}

void UAskirAnimationPlaybackComponent::TickComponent(float DeltaTime, ELevelTick Type,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, Type, ThisTickFunction);
    if (bVisualInstalled) UpdatePlayback(DeltaTime);
}

void UAskirAnimationPlaybackComponent::UpdatePlayback(float DeltaTime)
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    USkeletalMeshComponent* Mesh = Character ? Character->GetMesh() : nullptr;
    if (!Mesh || Mesh->GetSkeletalMeshAsset() != InstalledMesh ||
        Mesh->GetAnimationMode() != EAnimationMode::AnimationSingleNode) return;
    const UAskirAnimationStateComponent* Animation = State.Get();
    const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    const EAskirActionPose Action = Animation ? Animation->Action : EAskirActionPose::None;
    const FName Wanted = SelectClip(Action, Animation && Animation->bIsDead,
        Animation && Animation->bIsBlocking, Movement && Movement->IsFalling(),
        Character->GetVelocity().Size2D(), Animation && Animation->bIsSprinting, RunSpeedThreshold);
    const TObjectPtr<UAnimSequence>* Reference = LoadedClips.Find(Wanted);
    UAnimSequence* Sequence = Reference ? Reference->Get() : nullptr;
    if (!Sequence) return;
    const int32 ActionSequence = Animation ? Animation->ActionSequence : 0;
    const bool bLoop = IsLoopingClip(Wanted);
    if (CurrentClip != Wanted || (!bLoop && ActionSequence != LastActionSequence))
    {
        Mesh->PlayAnimation(Sequence, bLoop);
        CurrentClip = Wanted;
        PlaybackElapsed = 0.f;
        bFinalFrameHeld = false;
        if (State.IsValid() && State->IsCombatAction())
            State->TransientActionSeconds = FMath::Max(0.05f, Sequence->GetPlayLength());
    }
    LastActionSequence = ActionSequence;
    PlaybackElapsed += FMath::Max(0.f, DeltaTime);
    // Explicitly freeze the final frame of terminal/airborne clips, including death after load.
    if (!bLoop && !bFinalFrameHeld && PlaybackElapsed >= Sequence->GetPlayLength())
    {
        if (UAnimSingleNodeInstance* Node = Mesh->GetSingleNodeInstance())
        {
            Node->SetPosition(Sequence->GetPlayLength(), false);
            Node->SetPlaying(false);
            bFinalFrameHeld = true;
        }
    }
}
