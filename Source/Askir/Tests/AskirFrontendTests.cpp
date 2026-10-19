#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Frontend/AskirFrontendSubsystem.h"
#include "Game/AskirGameMode.h"
#include "Game/AskirPlayerController.h"
#include "Characters/AskirCharacter.h"
#include "Tests/AskirAutomationWorldFixture.h"
#include "Engine/GameInstance.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirFrontendUnreadyWorldTest,"Askir.Frontend.UnreadyWorldCannotDiscardActiveSession",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirFrontendUnreadyWorldTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    TStrongObjectPtr<UGameInstance> Instance(NewObject<UGameInstance>());
    TStrongObjectPtr<UAskirFrontendSubsystem> Frontend(NewObject<UAskirFrontendSubsystem>(Instance.Get()));
    auto* Mode=Fixture.Spawn<AAskirGameMode>();
    if(!TestNotNull(TEXT("Native game mode"),Mode))return false;
    TestFalse(TEXT("No session is invented on subsystem construction"),Frontend->HasActiveSession());
    TestFalse(TEXT("No travel request is invented"),Frontend->IsFreshStartPending());
    Frontend->MarkSessionActive();
    TestFalse(TEXT("Unready world cannot start a new session even after confirmation"),Frontend->StartNewGame(Mode,true));
    TestTrue(TEXT("Rejected new game preserves existing session"),Frontend->HasActiveSession());
    TestFalse(TEXT("Rejected new game cannot leave a busy menu"),Frontend->IsFreshStartPending());
    TestFalse(TEXT("No target world can commit without a queued request"),Frontend->CommitFreshStart(Mode));
    TestFalse(TEXT("Null mode cannot start a game"),Frontend->StartNewGame(nullptr,true));
    TestFalse(TEXT("Null mode cannot continue a game"),Frontend->ContinueGame(nullptr));
    TestTrue(TEXT("All failed actions retain the session"),Frontend->HasActiveSession());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirFrontendHeadlessTest,"Askir.Frontend.HeadlessControllerDoesNotPauseStartup",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirFrontendHeadlessTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    auto* Player=Fixture.Spawn<AAskirCharacter>();
    auto* Controller=Fixture.Spawn<AAskirPlayerController>();
    if(!TestNotNull(TEXT("Native player"),Player)||!TestNotNull(TEXT("Headless controller"),Controller))return false;
    Controller->Possess(Player);
    TestNull(TEXT("Automation controller has no real local viewport"),Controller->GetLocalPlayer());
    TestFalse(TEXT("Title bootstrap does not create a headless menu"),Controller->IsMenuOpen());
    TestFalse(TEXT("No frontend startup input lock for headless tests"),Controller->IsMoveInputIgnored());
    TestFalse(TEXT("No frontend camera lock for headless tests"),Controller->IsLookInputIgnored());
    TestTrue(TEXT("Native gameplay smoke tests remain possible"),Player->CanUseGameplayInput());
    return true;
}
#endif
