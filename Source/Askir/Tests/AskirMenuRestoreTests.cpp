#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Characters/AskirCharacter.h"
#include "Game/AskirGameMode.h"
#include "Game/AskirPlayerController.h"
#include "Save/AskirSaveGame.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirMenuRestoreTest,"Askir.Persistence.RestoreInvalidatesOpenMenus",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirMenuRestoreTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    auto* Player=Fixture.Spawn<AAskirCharacter>(FVector(0.f,0.f,96.f));
    auto* Controller=Fixture.Spawn<AAskirPlayerController>();
    auto* Mode=Fixture.Spawn<AAskirGameMode>();
    if(!TestNotNull(TEXT("Spawned player"),Player)||!TestNotNull(TEXT("Spawned controller"),Controller)||!TestNotNull(TEXT("Spawned game mode"),Mode))return false;
    Controller->Possess(Player);
    auto* Saved=Mode->CaptureSnapshot();
    if(!TestNotNull(TEXT("Capture complete valid player snapshot"),Saved))return false;
    const int32 Gold=Saved->SavedPlayerGold;
    Controller->SetMenuInput(true,false);
    TestTrue(TEXT("Outgoing menu context is open"),Controller->IsMenuOpen());
    Saved->SavedPlayerGold=-1;
    TestFalse(TEXT("Invalid snapshot must fail without closing current menu"),Mode->RestoreSnapshot(Saved,false));
    TestTrue(TEXT("Rejected restore preserves current input context"),Controller->IsMenuOpen());
    Saved->SavedPlayerGold=Gold;
    TestTrue(TEXT("Valid same-map restore succeeds"),Mode->RestoreSnapshot(Saved,false));
    TestFalse(TEXT("Restored timeline closes old menu context"),Controller->IsMenuOpen());
    Controller->SetMenuInput(true,false);
    TestTrue(TEXT("Repeated restore succeeds"),Mode->RestoreSnapshot(Saved,false));
    TestFalse(TEXT("Repeated restore also invalidates the outgoing menu"),Controller->IsMenuOpen());
    return true;
}
#endif
