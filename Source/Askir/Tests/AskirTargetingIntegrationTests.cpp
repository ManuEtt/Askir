#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Combat/AskirCombatComponent.h"
#include "Combat/AskirGuardComponent.h"
#include "Magic/AskirMagicComponent.h"
#include "Magic/AskirSpellProjectile.h"
#include "Components/BoxComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirMeleeTargetPolicyIntegrationTest,
    "Askir.Combat.V82.MeleeProtectsCivilianAndResolvesOneHostileHit",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirMeleeTargetPolicyIntegrationTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Runtime physics world"),Fixture.World))return false;
    auto* Player=Fixture.Spawn<AAskirCharacter>(FVector(0,0,96));
    auto* Controller=Fixture.Spawn<APlayerController>();
    auto* NPC=Fixture.Spawn<AAskirNPC>(FVector(120,0,96));
    if(!TestNotNull(TEXT("Player"),Player)||!TestNotNull(TEXT("NPC"),NPC)||!TestNotNull(TEXT("Controller"),Controller))return false;
    Controller->Possess(Player);Controller->SetControlRotation(FRotator::ZeroRotator);
    Player->GetCharacterMovement()->DisableMovement();NPC->GetCharacterMovement()->DisableMovement();
    Player->BaseAttackDamage=20.f;Player->AttackCooldown=0.f;NPC->DetectionRadius=0.f;
    NPC->bHostile=false;
    Player->Attack();
    TestTrue(TEXT("Friendly protection is checked at the real impact, not before the swing"),Player->HasPendingAttack());
    Player->ResolvePendingAttackImpact();
    TestEqual(TEXT("Civilian health is untouched by native melee"),NPC->CombatComponent->GetHealth(),100.f);
    NPC->bHostile=true;
    Player->Attack();Player->ResolvePendingAttackImpact();
    TestTrue(TEXT("Same physical target receives an actual hostile hit"),NPC->CombatComponent->GetHealth()<100.f);
    const float After=NPC->CombatComponent->GetHealth();
    Player->ResolvePendingAttackImpact();
    TestEqual(TEXT("Duplicate notify cannot apply another hit"),NPC->CombatComponent->GetHealth(),After);
    NPC->SetEncounterActive(false);
    Player->Attack();Player->ResolvePendingAttackImpact();
    TestEqual(TEXT("Deactivated encounter cannot take a queued swing"),NPC->CombatComponent->GetHealth(),After);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirFrontalGuardIntegrationTest,
    "Askir.Combat.V82.NativeAndLegacyGuardDoNotStackOrProtectTheRear",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirFrontalGuardIntegrationTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Runtime world"),Fixture.World))return false;
    auto* Player=Fixture.Spawn<AAskirCharacter>();
    if(!TestNotNull(TEXT("Defender"),Player))return false;
    Player->SetActorRotation(FRotator::ZeroRotator);
    FAskirDamageInfo Damage;Damage.Damage=20.f;Damage.HitDirection=FVector(-1,0,0);
    Player->StartBlock();Player->GuardComponent->StartGuard();
    const float LegacyStamina=Player->GuardComponent->Stamina;
    Player->ReceiveAskirDamage(Damage);
    TestTrue(TEXT("Front hit is reduced once by native block"),FMath::IsNearlyEqual(Player->CombatComponent->GetHealth(),92.f));
    TestEqual(TEXT("Legacy guard does not charge a second stamina pool"),Player->GuardComponent->Stamina,LegacyStamina);
    Player->CombatComponent->ResetHealth();Player->Stamina=100.f;
    Damage.HitDirection=FVector(1,0,0);Player->ReceiveAskirDamage(Damage);
    TestEqual(TEXT("Rear hit bypasses both guard paths"),Player->CombatComponent->GetHealth(),80.f);
    TestEqual(TEXT("Rear hit spends no native block stamina"),Player->Stamina,100.f);
    Player->CombatComponent->ResetHealth();Player->Stamina=0.f;Damage.HitDirection=FVector(-1,0,0);
    const float BeforeFailedBlock=Player->GuardComponent->Stamina;
    Player->ReceiveAskirDamage(Damage);
    TestEqual(TEXT("Exhausted native block cannot fall back to a second guard pool"),Player->CombatComponent->GetHealth(),80.f);
    TestEqual(TEXT("Failed native guard does not charge legacy stamina"),Player->GuardComponent->Stamina,BeforeFailedBlock);
    TestFalse(TEXT("Insufficient native stamina breaks native block"),Player->IsBlocking());
    Player->StopBlock();Player->CombatComponent->ResetHealth();
    Damage.HitDirection=FVector(-1,0,0);Player->CombatComponent->ApplyDamage(Damage);
    TestTrue(TEXT("Legacy-only front guard still works"),Player->CombatComponent->GetHealth()>80.f);
    Player->CombatComponent->ResetHealth();Damage.DamageType=EAskirDamageType::TrueDamage;
    Player->CombatComponent->ApplyDamage(Damage);
    TestEqual(TEXT("True damage bypasses legacy guard"),Player->CombatComponent->GetHealth(),80.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirSpellLaunchCoverTest,
    "Askir.Magic.V82.NearWallCannotBeSkippedByProjectileSpawnOffset",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirSpellLaunchCoverTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Runtime physics world"),Fixture.World))return false;
    auto* Player=Fixture.Spawn<AAskirCharacter>(FVector(0,0,96));
    auto* Controller=Fixture.Spawn<APlayerController>();
    if(!TestNotNull(TEXT("Caster"),Player)||!TestNotNull(TEXT("Controller"),Controller))return false;
    Controller->Possess(Player);Controller->SetControlRotation(FRotator::ZeroRotator);
    Player->GetCharacterMovement()->DisableMovement();
    AActor* Wall=Fixture.Spawn<AActor>();
    auto* Collision=Fixture.AddComponent<UBoxComponent>(Wall);Wall->SetRootComponent(Collision);
    Collision->SetBoxExtent(FVector(3,100,100));Collision->SetCollisionProfileName(TEXT("BlockAll"));
    Wall->SetActorLocation(FVector(35,0,136));
    auto* Magic=Player->MagicComponent;Magic->SelectSpell(TEXT("feuer_1"));
    const float ManaBefore=Magic->GetMana();
    TestFalse(TEXT("Wall between hand and nominal muzzle rejects spawn"),Magic->CastSelected());
    TestEqual(TEXT("Rejected launch consumes no mana"),Magic->GetMana(),ManaBefore);
    TestFalse(TEXT("Rejected launch installs no cooldown"),Magic->CaptureSnapshot().CooldownsRemaining.Contains(TEXT("feuer_1")));
    int32 Projectiles=0;
    for(TActorIterator<AAskirSpellProjectile> It(Fixture.World);It;++It)++Projectiles;
    TestEqual(TEXT("No projectile appears on the far side of cover"),Projectiles,0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirProjectileGuardBearingTest,
    "Askir.Magic.V82.ProjectileGuardUsesFlightDirectionAfterCasterMoves",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirProjectileGuardBearingTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Runtime physics world"),Fixture.World))return false;
    auto* Player=Fixture.Spawn<AAskirCharacter>(FVector(0,0,96));
    auto* Controller=Fixture.Spawn<APlayerController>();
    auto* Caster=Fixture.Spawn<AAskirNPC>(FVector(500,0,96));
    if(!TestNotNull(TEXT("Defender"),Player)||!TestNotNull(TEXT("Caster"),Caster)||!TestNotNull(TEXT("Controller"),Controller))return false;
    Controller->Possess(Player);Player->SetActorRotation(FRotator::ZeroRotator);
    Player->GetCharacterMovement()->DisableMovement();Caster->GetCharacterMovement()->DisableMovement();
    Caster->bHostile=true;Player->StartBlock();
    auto* Projectile=Fixture.Spawn<AAskirSpellProjectile>(FVector(-120,0,136));
    if(!TestNotNull(TEXT("Actual projectile"),Projectile))return false;
    Projectile->InitializeSpell(Caster,AskirMagicRules::Element::Fire,20.f,FVector(1,0,0));
    Projectile->Tick(.1f);
    TestEqual(TEXT("Projectile arriving from behind bypasses front block even when caster is now in front"),Player->CombatComponent->GetHealth(),80.f);
    TestEqual(TEXT("Rear projectile spends no block stamina"),Player->Stamina,100.f);
    return true;
}
#endif
