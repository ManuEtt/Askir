#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Characters/AskirCharacter.h"
#include "Combat/AskirCombatComponent.h"
#include "Animation/AskirAnimationStateComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirCharacterRestoreTest,"Askir.Save.CharacterRevivesMovementCollisionAndActions",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirCharacterRestoreTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Initialized runtime test world"),Fixture.World))return false;
    AAskirCharacter* Player=Fixture.Spawn<AAskirCharacter>(FVector(0,0,100));
    if(!TestNotNull(TEXT("Spawned character"),Player))return false;
    Player->StartBlock();
    Player->CombatComponent->Kill();
    TestTrue(TEXT("Death disables movement"),Player->GetCharacterMovement()->MovementMode==MOVE_None);
    TestTrue(TEXT("Death disables capsule collision"),Player->GetCapsuleComponent()->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
    Player->CombatComponent->RestoreHealth(50.f);
    Player->RestoreAfterLoad();
    TestFalse(TEXT("Saved living character revives"),Player->IsDead());
    TestTrue(TEXT("Saved living character walks"),Player->GetCharacterMovement()->MovementMode==MOVE_Walking);
    TestTrue(TEXT("Saved living character collides"),Player->GetCapsuleComponent()->GetCollisionEnabled()==ECollisionEnabled::QueryAndPhysics);
    TestFalse(TEXT("Death animation flag resets"),Player->AnimationState->bIsDead);
    TestFalse(TEXT("Old block input resets"),Player->AnimationState->bIsBlocking);
    TestFalse(TEXT("Old pending attack resets"),Player->HasPendingAttack());
    Player->Attack();
    TestTrue(TEXT("Revived character can begin a new attack"),Player->HasPendingAttack());
    Player->CombatComponent->RestoreHealth(0.f);Player->RestoreAfterLoad();
    TestTrue(TEXT("Saved dead character remains unable to walk"),Player->GetCharacterMovement()->MovementMode==MOVE_None);
    TestTrue(TEXT("Saved dead character preserves death pose"),Player->AnimationState->bIsDead);
    TestFalse(TEXT("Saved dead character cannot replay an old attack"),Player->HasPendingAttack());
    return true;
}
#endif
