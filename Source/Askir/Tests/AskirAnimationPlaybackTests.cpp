#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Animation/AskirAnimationPlaybackComponent.h"
#include "Characters/AskirCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Tests/AskirAutomationWorldFixture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirAnimationPriorityTest,
    "Askir.Animation.ActionsOverrideLocomotionAndDeathIsTerminal",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAskirAnimationPriorityTest::RunTest(const FString& Parameters)
{
    using Playback = UAskirAnimationPlaybackComponent;
    auto Clip = [](EAskirActionPose Pose, bool bDead = false) {
        return Playback::SelectClip(Pose, bDead, true, true, 700.f, true, 300.f);
    };
    TestEqual(TEXT("A dead casting actor uses death"), Clip(EAskirActionPose::Cast, true), FName(TEXT("Death")));
    TestEqual(TEXT("Death pose overrides movement even before flag update"), Clip(EAskirActionPose::Death), FName(TEXT("Death")));
    TestEqual(TEXT("Light attack overrides locomotion and block flag"), Clip(EAskirActionPose::LightAttack), FName(TEXT("LightAttack")));
    TestEqual(TEXT("Heavy attack overrides locomotion"), Clip(EAskirActionPose::HeavyAttack), FName(TEXT("HeavyAttack")));
    TestEqual(TEXT("Dodge is independent of airborne movement"), Clip(EAskirActionPose::Dodge), FName(TEXT("Dodge")));
    TestEqual(TEXT("Hit reaction overrides block"), Clip(EAskirActionPose::HitReact), FName(TEXT("Hit")));
    TestEqual(TEXT("Cast has an actual imported clip"), Clip(EAskirActionPose::Cast), FName(TEXT("Cast")));
    TestEqual(TEXT("Block wins over locomotion"), Clip(EAskirActionPose::None), FName(TEXT("Block")));
    TestFalse(TEXT("Death never loops"), Playback::IsLoopingClip(TEXT("Death")));
    TestFalse(TEXT("Cast never loops"), Playback::IsLoopingClip(TEXT("Cast")));
    TestTrue(TEXT("Block loops until released"), Playback::IsLoopingClip(TEXT("Block")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirLocomotionClipTest,
    "Askir.Animation.LocomotionSelectsWalkRunSprintAndAirborneClips",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAskirLocomotionClipTest::RunTest(const FString& Parameters)
{
    using Playback = UAskirAnimationPlaybackComponent;
    auto Clip = [](float Speed, bool bSprint = false, bool bAir = false) {
        return Playback::SelectClip(EAskirActionPose::None, false, false, bAir, Speed, bSprint, 300.f);
    };
    TestEqual(TEXT("Standing sprint input still uses idle"), Clip(0.f, true), FName(TEXT("Idle")));
    TestEqual(TEXT("Slow movement uses walk"), Clip(150.f), FName(TEXT("Walk")));
    TestEqual(TEXT("Run begins at the configured threshold"), Clip(300.f), FName(TEXT("Run")));
    TestEqual(TEXT("Active sprint uses sprint"), Clip(700.f, true), FName(TEXT("Sprint")));
    TestEqual(TEXT("Airborne movement uses jump"), Clip(700.f, true, true), FName(TEXT("Jump")));
    TestTrue(TEXT("Idle loops without repeated restart"), Playback::IsLoopingClip(TEXT("Idle")));
    TestTrue(TEXT("Walk loops"), Playback::IsLoopingClip(TEXT("Walk")));
    TestTrue(TEXT("Run loops"), Playback::IsLoopingClip(TEXT("Run")));
    TestTrue(TEXT("Sprint loops"), Playback::IsLoopingClip(TEXT("Sprint")));
    TestFalse(TEXT("Jump holds its final pose instead of looping"), Playback::IsLoopingClip(TEXT("Jump")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirMissingVisualFallbackTest,
    "Askir.Animation.MissingImportedMeshPreservesExistingVisual",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAskirMissingVisualFallbackTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if (!TestNotNull(TEXT("Initialized runtime test world"), Fixture.World)) return false;
    AAskirCharacter* Character = Fixture.Spawn<AAskirCharacter>(FVector(0,0,100));
    if (!TestNotNull(TEXT("Spawned character"), Character)) return false;
    UAskirAnimationPlaybackComponent* Playback = Fixture.AddComponent<UAskirAnimationPlaybackComponent>(Character);
    if (!TestNotNull(TEXT("Playback component registered"), Playback)) return false;
    Playback->bInstallOnBeginPlay = false;
    USkeletalMesh* PreviousMesh = Character->GetMesh()->GetSkeletalMeshAsset();
    Playback->CharacterMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/Askir/AbsentV78/SK_Missing.SK_Missing")));
    TestFalse(TEXT("Missing source-only content cannot install a visual"), Playback->TryInstallVisual());
    TestTrue(TEXT("The pre-existing visual is preserved"), Character->GetMesh()->GetSkeletalMeshAsset() == PreviousMesh);
    const FSoftObjectPath PreviousPath = Playback->CharacterMesh.ToSoftObjectPath();
    TestFalse(TEXT("Unsupported role cannot configure unsafe content paths"), Playback->ConfigureCharacterVisual(TEXT("Unknown/Role")));
    TestTrue(TEXT("Unsupported role leaves configuration intact"), Playback->CharacterMesh.ToSoftObjectPath() == PreviousPath);
    return true;
}
#endif
