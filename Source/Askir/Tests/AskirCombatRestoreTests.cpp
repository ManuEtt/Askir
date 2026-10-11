#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Combat/AskirCombatComponent.h"
#include "GameFramework/Actor.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirCombatRestoreTest,"Askir.Combat.RestoreDoesNotReplayDeath",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirCombatRestoreTest::RunTest(const FString& Parameters)
{
 AActor* Owner=NewObject<AActor>(GetTransientPackage());
 UAskirCombatComponent* Combat=NewObject<UAskirCombatComponent>(Owner);
 Combat->RestoreHealth(0.f);
 TestTrue(TEXT("Restored dead actor remains dead"),Combat->IsDead());
 TestEqual(TEXT("Restored dead actor has zero health"),Combat->GetHealth(),0.f);
 TestNull(TEXT("Restoration cannot preserve kill attribution"),Combat->GetLastDamageCauser());
 Combat->ResetHealth();
 TestFalse(TEXT("Reset revives actor"),Combat->IsDead());
 TestTrue(TEXT("Reset restores positive health"),Combat->GetHealth()>0.f);
 return true;
}
#endif
