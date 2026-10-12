#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Combat/AskirCombatComponent.h"
#include "Characters/AskirNPC.h"
#include "Characters/AskirNPCAdvanced.h"
#include "AI/AskirEnemyAIController.h"
#include "Animation/AskirAnimationStateComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Engine/Engine.h"
#include "Engine/URL.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace AskirCombatTests
{
using FWorldFixture=FAskirAutomationTestWorld;
template<class T> T* AddComponent(AActor* Owner)
{
    T* Component=NewObject<T>(Owner);
    Owner->AddInstanceComponent(Component);
    Component->RegisterComponent();
    return Component;
}
FAskirDamageInfo Hit(AActor* Causer,float Amount)
{
    FAskirDamageInfo Damage;Damage.DamageCauser=Causer;Damage.Damage=Amount;return Damage;
}
UAskirQuestComponent* AddGuardianQuest(APawn* Pawn)
{
    UAskirQuestComponent* Quests=AddComponent<UAskirQuestComponent>(Pawn);
    UAskirQuestData* Quest=NewObject<UAskirQuestData>(Quests);
    Quest->QuestID=TEXT("CombatRegression");
    FAskirQuestObjective Objective;Objective.ObjectiveID=TEXT("Kill_ShadowGuardian");Objective.RequiredCount=2;
    Quest->Objectives.Add(Objective);Quests->StartQuest(Quest);
    return Quests;
}
int32 GuardianProgress(const UAskirQuestComponent* Quests)
{
    const TArray<FAskirQuestObjective> Objectives=Quests->GetObjectivesForQuest(TEXT("CombatRegression"));
    return Objectives.IsValidIndex(0)?Objectives[0].CurrentCount:-1;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirProjectileAttributionTest,"Askir.Combat.ProjectileInstigatorSurvivesSourceDestruction",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirProjectileAttributionTest::RunTest(const FString& Parameters)
{
    using namespace AskirCombatTests;
    FWorldFixture Fixture;
    if(!TestNotNull(TEXT("Runtime test world"),Fixture.World))return false;
    APawn* Player=Fixture.Spawn<APawn>();
    AActor* Target=Fixture.Spawn<AActor>();
    AActor* Projectile=Fixture.Spawn<AActor>();
    UAskirCombatComponent* Combat=AddComponent<UAskirCombatComponent>(Target);
    Projectile->SetInstigator(Player);
    Combat->ApplyDamage(Hit(Projectile,10.f));
    Projectile->Destroy();
    TestEqual(TEXT("Projectile credit resolves at hit time"),Combat->GetLastDamageCauser(),static_cast<AActor*>(Player));
    TestFalse(TEXT("Alive victim has no kill receipt"),Combat->IsKillAttributedTo(Player));
    Combat->Kill();
    TestTrue(TEXT("Recent player hit credits subsequent death"),Combat->IsKillAttributedTo(Player));
    Combat->ResetHealth();
    AActor* Weapon=Fixture.Spawn<AActor>();Weapon->SetOwner(Player);
    AActor* OwnedProjectile=Fixture.Spawn<AActor>();OwnedProjectile->SetOwner(Weapon);
    Combat->ApplyDamage(Hit(OwnedProjectile,200.f));
    TestTrue(TEXT("Projectile ownership chain resolves to responsible pawn"),Combat->IsKillAttributedTo(Player));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirEnvironmentAttributionTest,"Askir.Combat.EnvironmentAndExpiredHitsDoNotCreditPlayer",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirEnvironmentAttributionTest::RunTest(const FString& Parameters)
{
    using namespace AskirCombatTests;
    FWorldFixture Fixture;
    if(!TestNotNull(TEXT("Runtime test world"),Fixture.World))return false;
    APawn* Player=Fixture.Spawn<APawn>();
    UAskirCombatComponent* Combat=AddComponent<UAskirCombatComponent>(Fixture.Spawn<AActor>());
    Combat->ApplyDamage(Hit(Player,5.f));
    TestEqual(TEXT("Direct player hit"),Combat->GetLastDamageCauser(),static_cast<AActor*>(Player));
    Combat->ApplyDamage(Hit(nullptr,200.f));
    TestTrue(TEXT("Environment lethal hit kills"),Combat->IsDead());
    TestFalse(TEXT("Environment lethal hit clears player credit"),Combat->IsKillAttributedTo(Player));
    Combat->ResetHealth();
    Combat->KillCreditWindowSeconds=.01f;
    Combat->ApplyDamage(Hit(Player,5.f));
    Fixture.World->Tick(LEVELTICK_All,.02f);
    Combat->Kill();
    TestFalse(TEXT("Expired positive hit cannot credit scripted death"),Combat->IsKillAttributedTo(Player));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirDodgeDamageTest,"Askir.Combat.DodgeAndInactiveEncounterRejectDamage",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirDodgeDamageTest::RunTest(const FString& Parameters)
{
    using namespace AskirCombatTests;
    FWorldFixture Fixture;
    if(!TestNotNull(TEXT("Runtime test world"),Fixture.World))return false;
    APawn* Player=Fixture.Spawn<APawn>();
    UAskirCombatComponent* Combat=AddComponent<UAskirCombatComponent>(Fixture.Spawn<AActor>());
    Combat->GrantDodgeInvulnerability(.2f);
    TestEqual(TEXT("Dodge rejects positive damage"),Combat->ApplyDamage(Hit(Player,15.f)),0.f);
    TestNull(TEXT("Dodged hit cannot become kill attribution"),Combat->GetLastDamageCauser());
    Combat->RestoreHealth(75.f);
    TestFalse(TEXT("Restore clears transient dodge immunity"),Combat->IsDodgeInvulnerable());
    Combat->SetDamageEnabled(false);
    TestEqual(TEXT("Inactive encounter rejects direct damage"),Combat->ApplyDamage(Hit(Player,100.f)),0.f);
    TestEqual(TEXT("Inactive encounter preserves health"),Combat->GetHealth(),75.f);
    Combat->SetDamageEnabled(true);
    TestEqual(TEXT("Damage result is actual health lost, capped at remaining health"),Combat->ApplyDamage(Hit(Player,100.f)),75.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirNPCRewardsTest,"Askir.Combat.NPCDeathRewardsOnceAndPlayerOnlyQuestCredit",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirNPCRewardsTest::RunTest(const FString& Parameters)
{
    using namespace AskirCombatTests;
    FWorldFixture Fixture;
    if(!TestNotNull(TEXT("Runtime test world"),Fixture.World))return false;
    APawn* Player=Fixture.Spawn<APawn>();
    Fixture.Spawn<APlayerController>()->Possess(Player);
    UAskirProgressionComponent* Progression=AddComponent<UAskirProgressionComponent>(Player);
    UAskirQuestComponent* PlayerQuests=AddGuardianQuest(Player);
    AAskirNPC* Enemy=Fixture.Spawn<AAskirNPC>();
    Enemy->DefeatObjectiveID=TEXT("Kill_ShadowGuardian");Enemy->KillExperience=20;
    Enemy->CombatComponent->ApplyDamage(Hit(Player,200.f));
    TestEqual(TEXT("Player receives one enemy XP reward"),Progression->Progress.Experience,20);
    TestEqual(TEXT("Player receives one guardian objective"),GuardianProgress(PlayerQuests),1);
    // Deliberately replay the delegate to exercise the reward latch beyond Kill().
    Enemy->CombatComponent->OnDeath.Broadcast();
    TestEqual(TEXT("Death delegate replay grants no extra XP"),Progression->Progress.Experience,20);
    TestEqual(TEXT("Death delegate replay grants no extra quest credit"),GuardianProgress(PlayerQuests),1);
    APawn* AlliedNPC=Fixture.Spawn<APawn>();
    UAskirQuestComponent* NPCQuests=AddGuardianQuest(AlliedNPC);
    AAskirNPC* OtherEnemy=Fixture.Spawn<AAskirNPC>();OtherEnemy->DefeatObjectiveID=TEXT("Kill_ShadowGuardian");
    OtherEnemy->CombatComponent->ApplyDamage(Hit(AlliedNPC,200.f));
    TestEqual(TEXT("NPC kill does not advance player's quest"),GuardianProgress(PlayerQuests),1);
    TestEqual(TEXT("NPC kill does not advance NPC's player objective"),GuardianProgress(NPCQuests),0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirNPCEncounterRestoreTest,"Askir.Combat.NPCEncounterRestorePreservesReceiptAndRevives",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirNPCEncounterRestoreTest::RunTest(const FString& Parameters)
{
    using namespace AskirCombatTests;
    FWorldFixture Fixture;
    if(!TestNotNull(TEXT("Runtime test world"),Fixture.World))return false;
    APawn* Player=Fixture.Spawn<APawn>();
    UAskirProgressionComponent* Progression=AddComponent<UAskirProgressionComponent>(Player);
    AAskirNPC* Enemy=Fixture.Spawn<AAskirNPC>();
    Enemy->SetEncounterActive(false);
    Enemy->CombatComponent->ApplyDamage(Hit(Player,200.f));
    TestFalse(TEXT("Dormant guardian survives stray attack"),Enemy->IsDead());
    Enemy->RestoreEncounterState(true);
    Enemy->CombatComponent->OnDeath.Broadcast();
    TestEqual(TEXT("Restored corpse never replays rewards"),Progression->Progress.Experience,0);
    TestTrue(TEXT("Restored corpse is dead"),Enemy->IsDead());
    Enemy->RestoreEncounterState(false);
    TestFalse(TEXT("Unconsumed guardian becomes alive"),Enemy->IsDead());
    TestTrue(TEXT("Revived encounter is active"),Enemy->IsEncounterActive());
    TestEqual(TEXT("Movement resumes after restore"),Enemy->GetCharacterMovement()->MovementMode,static_cast<TEnumAsByte<EMovementMode>>(MOVE_Walking));
    TestFalse(TEXT("Animation death flag is cleared"),Enemy->AnimationState->bIsDead);
    AAskirNPCAdvanced* Advanced=Fixture.Spawn<AAskirNPCAdvanced>();
    TestNotNull(TEXT("Advanced NPC reuses inherited needs component"),Advanced->NeedsComponent.Get());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirEnemyAIReuseTest,"Askir.Combat.EnemyControllerReuseAndInactiveEncounter",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirEnemyAIReuseTest::RunTest(const FString& Parameters)
{
    using namespace AskirCombatTests;
    FWorldFixture Fixture;
    if(!TestNotNull(TEXT("Runtime test world"),Fixture.World))return false;
    AAskirNPC* Enemy=Fixture.Spawn<AAskirNPC>();
    AAskirEnemyAIController* AI=Fixture.Spawn<AAskirEnemyAIController>();
    AI->Possess(Enemy);
    AI->ForceInvestigate(FVector(300.f,0.f,0.f));
    TestEqual(TEXT("Explicit stimulus starts investigation"),AI->State,EAskirEnemyState::Investigate);
    Enemy->Attack();
    TestTrue(TEXT("Pawn has a pending delayed attack"),Enemy->HasPendingAttack());
    Enemy->SetEncounterActive(false);
    AI->Tick(.4f);
    TestFalse(TEXT("Encounter gating cancels the pawn damage timer"),Enemy->HasPendingAttack());
    TestFalse(TEXT("Encounter gating disables incoming damage"),Enemy->CombatComponent->IsDamageEnabled());
    AI->UnPossess();
    AAskirNPC* Replacement=Fixture.Spawn<AAskirNPC>();Replacement->SetActorLocation(FVector(400.f,0.f,0.f));
    AI->Possess(Replacement);
    TestEqual(TEXT("Reused controller resets state"),AI->State,EAskirEnemyState::Patrol);
    TestEqual(TEXT("Reused controller captures new home"),AI->HomeLocation,Replacement->GetActorLocation());
    return true;
}
#endif
