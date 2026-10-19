#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Combat/AskirTargetLockComponent.h"
#include "Combat/AskirCombatComponent.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

namespace
{
struct FTargetFixture : FAskirAutomationTestWorld
{
    AAskirCharacter* Player=nullptr;
    APlayerController* Controller=nullptr;
    UAskirTargetLockComponent* Lock=nullptr;
    FTargetFixture()
    {
        if(!World)return;
        Player=Spawn<AAskirCharacter>(FVector(0.,0.,96.));Controller=Spawn<APlayerController>();
        if(!Player||!Controller)return;
        Controller->Possess(Player);Controller->SetControlRotation(FRotator::ZeroRotator);
        Player->GetCharacterMovement()->DisableMovement();
        Lock=Player->FindComponentByClass<UAskirTargetLockComponent>();
        if(!Lock)Lock=AddComponent<UAskirTargetLockComponent>(Player);
    }
    AAskirNPC* Enemy(const FVector& Position)
    {
        AAskirNPC* NPC=Spawn<AAskirNPC>(Position);
        if(NPC){NPC->bHostile=true;NPC->SetEncounterActive(true);NPC->GetCharacterMovement()->DisableMovement();}
        return NPC;
    }
    AActor* Wall(const FVector& Position)
    {
        AActor* Actor=Spawn<AActor>(Position);if(!Actor)return nullptr;
        UBoxComponent* Box=AddComponent<UBoxComponent>(Actor);Actor->SetRootComponent(Box);
        Box->SetBoxExtent(FVector(20.,120.,180.));Box->SetCollisionProfileName(TEXT("BlockAll"));
        Actor->SetActorLocation(Position);return Actor;
    }
    bool IsReady() const { return World&&Player&&Controller&&Lock; }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirTargetAcquisitionTest,"Askir.Combat.TargetLock.AcquisitionPolicyAndRotationRestore",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirTargetAcquisitionTest::RunTest(const FString& Parameters)
{
    FTargetFixture Fixture;if(!TestTrue(TEXT("Controlled runtime fixture"),Fixture.IsReady()))return false;
    AAskirNPC* Front=Fixture.Enemy(FVector(500.,0.,96.));
    AAskirNPC* Civilian=Fixture.Enemy(FVector(160.,120.,96.));Civilian->bHostile=false;
    AAskirNPC* Dormant=Fixture.Enemy(FVector(250.,0.,96.));Dormant->SetEncounterActive(false);
    Fixture.Enemy(FVector(-150.,0.,96.)); // Outside the acquisition cone.
    Fixture.Enemy(FVector(1500.,0.,96.)); // Outside the acquisition radius.
    UCharacterMovementComponent* Movement=Fixture.Player->GetCharacterMovement();
    Movement->bOrientRotationToMovement=true;Movement->bUseControllerDesiredRotation=true;Fixture.Player->bUseControllerRotationYaw=true;
    TestTrue(TEXT("A visible hostile target can be acquired"),Fixture.Lock->ToggleLock());
    TestEqual(TEXT("Civilian/dormant/rear/distant actors are excluded"),Fixture.Lock->GetLockedTarget(),static_cast<AActor*>(Front));
    TestFalse(TEXT("Locked movement strafes rather than following movement yaw"),Movement->bOrientRotationToMovement);
    TestFalse(TEXT("Facing is explicitly controlled by lock component"),Fixture.Player->bUseControllerRotationYaw);
    TestFalse(TEXT("Controller desired rotation cannot override target-facing yaw"),Movement->bUseControllerDesiredRotation);
    TestFalse(TEXT("Toggling an existing lock releases it"),Fixture.Lock->ToggleLock());
    TestFalse(TEXT("Released lock has no target"),Fixture.Lock->IsLocked());
    TestTrue(TEXT("Original movement orientation is restored"),Movement->bOrientRotationToMovement);
    TestTrue(TEXT("Original controller-yaw flag is restored"),Fixture.Player->bUseControllerRotationYaw);
    TestTrue(TEXT("Original desired-rotation flag is restored"),Movement->bUseControllerDesiredRotation);
    Movement->bOrientRotationToMovement=false;Fixture.Player->bUseControllerRotationYaw=false;
    Fixture.Lock->ToggleLock();Fixture.Lock->ClearLock();
    TestFalse(TEXT("Custom original movement flags survive release"),Movement->bOrientRotationToMovement);
    TestFalse(TEXT("Custom original yaw flags survive release"),Fixture.Player->bUseControllerRotationYaw);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirTargetCycleTest,"Askir.Combat.TargetLock.DirectionalCycleAndSingleCandidate",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirTargetCycleTest::RunTest(const FString& Parameters)
{
    FTargetFixture Fixture;if(!TestTrue(TEXT("Controlled runtime fixture"),Fixture.IsReady()))return false;
    AAskirNPC* Center=Fixture.Enemy(FVector(500.,0.,96.));
    AAskirNPC* Right=Fixture.Enemy(FVector(500.,300.,96.));
    AAskirNPC* Left=Fixture.Enemy(FVector(500.,-300.,96.));
    TestTrue(TEXT("Initial center lock"),Fixture.Lock->ToggleLock());
    TestEqual(TEXT("Camera-center candidate wins equal-distance side targets"),Fixture.Lock->GetLockedTarget(),static_cast<AActor*>(Center));
    TestTrue(TEXT("Positive cycle chooses camera right"),Fixture.Lock->CycleTarget(1));
    TestEqual(TEXT("Right target selected"),Fixture.Lock->GetLockedTarget(),static_cast<AActor*>(Right));
    TestTrue(TEXT("Negative cycle returns to center"),Fixture.Lock->CycleTarget(-1));
    TestEqual(TEXT("Center restored"),Fixture.Lock->GetLockedTarget(),static_cast<AActor*>(Center));
    TestTrue(TEXT("Negative cycle chooses camera left"),Fixture.Lock->CycleTarget(-1));
    TestEqual(TEXT("Left target selected"),Fixture.Lock->GetLockedTarget(),static_cast<AActor*>(Left));
    Center->SetEncounterActive(false);Right->SetEncounterActive(false);
    TestFalse(TEXT("Single remaining candidate is retained without false cycling success"),Fixture.Lock->CycleTarget(1));
    TestEqual(TEXT("No-candidate cycle preserves lock"),Fixture.Lock->GetLockedTarget(),static_cast<AActor*>(Left));
    TestFalse(TEXT("Zero direction does not cycle"),Fixture.Lock->CycleTarget(0));
    Fixture.Lock->ClearLock();Right->SetEncounterActive(true);Fixture.Controller->SetControlRotation(FRotator::ZeroRotator);
    TestTrue(TEXT("Symmetric side candidates can be acquired"),Fixture.Lock->ToggleLock());
    AActor* StableWinner=Left->GetPathName()<Right->GetPathName()?static_cast<AActor*>(Left):static_cast<AActor*>(Right);
    TestEqual(TEXT("Equal geometry uses stable actor path rather than iterator order"),Fixture.Lock->GetLockedTarget(),StableWinner);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirTargetOcclusionTest,"Askir.Combat.TargetLock.CoverBlocksAimImmediatelyAndReleasesAfterGrace",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirTargetOcclusionTest::RunTest(const FString& Parameters)
{
    FTargetFixture Fixture;if(!TestTrue(TEXT("Controlled runtime fixture"),Fixture.IsReady()))return false;
    Fixture.Enemy(FVector(500.,0.,96.));
    AActor* Cover=Fixture.Wall(FVector(250.,0.,100.));
    TestFalse(TEXT("Target behind blocking geometry cannot be acquired"),Fixture.Lock->ToggleLock());
    Cover->SetActorEnableCollision(false);
    TestTrue(TEXT("Removing cover permits acquisition"),Fixture.Lock->ToggleLock());
    FVector Aim=FVector::ZeroVector;
    TestTrue(TEXT("Visible target produces fresh direct aim"),Fixture.Lock->GetAimDirection(Fixture.Player->GetActorLocation()+FVector(0.,0.,40.),Aim));
    TestTrue(TEXT("Aim is normalized and points forward"),Aim.IsNormalized()&&Aim.X>.99);
    Cover->SetActorEnableCollision(true);
    TestFalse(TEXT("Eye visibility updates without a targeting tick"),Fixture.Lock->IsTargetVisible());
    TestFalse(TEXT("New projectile aim is denied immediately behind cover"),Fixture.Lock->GetAimDirection(Fixture.Player->GetActorLocation()+FVector(0.,0.,40.),Aim));
    Fixture.World->Tick(LEVELTICK_All,.15f);
    TestTrue(TEXT("Brief LOS interruption retains the selected target"),Fixture.Lock->IsLocked());
    Fixture.World->Tick(LEVELTICK_All,.25f);
    TestFalse(TEXT("LOS grace expires using world time"),Fixture.Lock->IsLocked());
    TestTrue(TEXT("Occlusion release restores normal movement orientation"),Fixture.Player->GetCharacterMovement()->bOrientRotationToMovement);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirTargetInvalidationTest,"Askir.Combat.TargetLock.StateChangesReleaseWithoutStaleAim",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirTargetInvalidationTest::RunTest(const FString& Parameters)
{
    FTargetFixture Fixture;if(!TestTrue(TEXT("Controlled runtime fixture"),Fixture.IsReady()))return false;
    AAskirNPC* Enemy=Fixture.Enemy(FVector(500.,0.,96.));
    TestTrue(TEXT("Acquire live target"),Fixture.Lock->ToggleLock());
    Enemy->bHostile=false;
    TestNull(TEXT("Friendly target invalidated before next component tick"),Fixture.Lock->GetLockedTarget());
    Fixture.Lock->TickComponent(.01f,LEVELTICK_All,nullptr);
    TestTrue(TEXT("Friendly transition restores rotation flags"),Fixture.Player->GetCharacterMovement()->bOrientRotationToMovement);
    Enemy->bHostile=true;Fixture.Lock->ToggleLock();
    Enemy->SetActorLocation(FVector(1700.,0.,96.));
    TestTrue(TEXT("Retention radius avoids acquisition-edge flicker"),Fixture.Lock->IsLocked());
    Enemy->SetActorLocation(FVector(1900.,0.,96.));
    TestFalse(TEXT("Leaving retention radius invalidates immediately"),Fixture.Lock->IsLocked());
    Fixture.Lock->ClearLock();Enemy->SetActorLocation(FVector(500.,0.,96.));Fixture.Lock->ToggleLock();
    Enemy->CombatComponent->RestoreHealth(0.f);
    TestFalse(TEXT("Dead target cannot remain locked"),Fixture.Lock->IsLocked());
    Fixture.Lock->ClearLock();Enemy->CombatComponent->ResetHealth();Fixture.Lock->ToggleLock();
    Fixture.Player->CancelGameplayActions();
    // The character integration clears targeting when any menu/travel/load owns input.
    TestFalse(TEXT("Action cancellation clears transient target references"),Fixture.Lock->IsLocked());
    TestTrue(TEXT("Input remains usable after action cancellation"),Fixture.Lock->ToggleLock());
    Enemy->Destroy();
    TestNull(TEXT("Destroyed actor cannot remain a target"),Fixture.Lock->GetLockedTarget());
    Fixture.Lock->TickComponent(.01f,LEVELTICK_All,nullptr);
    TestTrue(TEXT("Destroyed target restores movement orientation"),Fixture.Player->GetCharacterMovement()->bOrientRotationToMovement);
    return true;
}
#endif
