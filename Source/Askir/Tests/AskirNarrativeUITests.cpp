#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Game/AskirPlayerController.h"
#include "Mission/AskirDeliveryMissionComponent.h"
#include "NPC/AskirDialogueComponent.h"
#include "Social/AskirReputationComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirNarrativeComponentsTest,"Askir.Narrative.NativePlayerAndNPCComponentsAreUnique",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirNarrativeComponentsTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    auto* Player=Fixture.Spawn<AAskirCharacter>();
    auto* Resident=Fixture.Spawn<AAskirNPC>(FVector(500.f,0.f,0.f));
    if(!TestNotNull(TEXT("Player default components construct"),Player)||!TestNotNull(TEXT("NPC inherits narrative components without duplicate subobjects"),Resident))return false;
    for(AAskirCharacter* Character:{Player,static_cast<AAskirCharacter*>(Resident)})
    {
        TArray<UAskirDialogueComponent*> Dialogues;Character->GetComponents(Dialogues);
        TArray<UAskirReputationComponent*> Reputations;Character->GetComponents(Reputations);
        TArray<UAskirDeliveryMissionComponent*> Deliveries;Character->GetComponents(Deliveries);
        TestEqual(TEXT("Exactly one inherited dialogue component"),Dialogues.Num(),1);
        TestEqual(TEXT("Exactly one reputation component"),Reputations.Num(),1);
        TestEqual(TEXT("Exactly one delivery mission component"),Deliveries.Num(),1);
        TestEqual(TEXT("Save lookup resolves the same dialogue component"),Character->FindComponentByClass<UAskirDialogueComponent>(),Character->DialogueComponent.Get());
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirNarrativeMenuInputTest,"Askir.Narrative.HeadlessMenuCancelsCombatAndRestoresInput",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirNarrativeMenuInputTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    auto* Player=Fixture.Spawn<AAskirCharacter>();auto* Controller=Fixture.Spawn<AAskirPlayerController>();
    if(!TestNotNull(TEXT("Player"),Player)||!TestNotNull(TEXT("Headless native controller"),Controller))return false;
    Controller->Possess(Player);
    TestTrue(TEXT("Gameplay ready before opening dialogue"),Player->CanUseGameplayInput());
    Player->Attack();TestTrue(TEXT("Committed combat action pending"),Player->HasPendingAttack());
    Controller->SetMenuInput(true,false);
    TestFalse(TEXT("Menu cancels pending impact"),Player->HasPendingAttack());
    TestFalse(TEXT("Dialogue blocks gameplay actions"),Player->CanUseGameplayInput());
    TestTrue(TEXT("Menu ignores movement"),Controller->IsMoveInputIgnored());
    TestTrue(TEXT("Menu ignores camera look"),Controller->IsLookInputIgnored());
    Player->Attack();TestFalse(TEXT("An open menu cannot schedule another attack"),Player->HasPendingAttack());
    Controller->CloseAnyMenu();
    TestFalse(TEXT("Closing missing HUD still clears menu state"),Controller->IsMenuOpen());
    TestFalse(TEXT("Movement restored"),Controller->IsMoveInputIgnored());
    TestFalse(TEXT("Look restored"),Controller->IsLookInputIgnored());
    TestTrue(TEXT("Gameplay restored after closing"),Player->CanUseGameplayInput());
    return true;
}
#endif
