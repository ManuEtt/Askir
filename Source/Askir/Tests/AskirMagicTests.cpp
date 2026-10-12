#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Magic/AskirMagicComponent.h"
#include "Magic/AskirSpellProjectile.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Combat/AskirCombatComponent.h"
#include "Save/AskirSaveGame.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirMagicShieldTest,"Askir.Magic.ShieldManaCooldownAndActualDamage",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirMagicShieldTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;if(!TestNotNull(TEXT("World"),Fixture.World))return false;
    auto* Player=Fixture.Spawn<AAskirCharacter>();auto* Controller=Fixture.Spawn<APlayerController>();Controller->Possess(Player);
    auto* Magic=Player->MagicComponent;
    TestTrue(TEXT("Select canonical shield"),Magic->SelectSpell(TEXT("feuer_2")));
    TestTrue(TEXT("Cast actual shield"),Magic->CastSelected());
    TestEqual(TEXT("Shield consumes 20 mana"),Magic->GetMana(),80.f);
    TestEqual(TEXT("Shield absorbs 30"),Magic->GetShield(),30.f);
    TestFalse(TEXT("Same-frame recast blocked"),Magic->CastSelected());
    FAskirDamageInfo Damage;Damage.Damage=40.f;Damage.DamageType=EAskirDamageType::Physical;
    Player->ReceiveAskirDamage(Damage);
    TestEqual(TEXT("Only unshielded 10 reaches health"),Player->CombatComponent->GetHealth(),90.f);
    TestEqual(TEXT("Shield exhausted"),Magic->GetShield(),0.f);
    TestTrue(TEXT("Capture retains cooldown"),Magic->CaptureSnapshot().CooldownsRemaining.Contains(TEXT("feuer_2")));
    FAskirMagicSnapshot S=Magic->CaptureSnapshot();
    TestTrue(TEXT("Snapshot accepted"),Magic->CanRestoreSnapshot(S));TestTrue(TEXT("Restore succeeds"),Magic->RestoreSnapshot(S));
    TestFalse(TEXT("Load cannot bypass cooldown"),Magic->CastSelected());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirMagicSaveTest,"Askir.Magic.SnapshotValidationAndSerialization",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirMagicSaveTest::RunTest(const FString& Parameters)
{
    FAskirMagicSnapshot S;S.Mana=65.f;S.SelectedSpellID=TEXT("eis_1");S.CooldownsRemaining.Add(TEXT("eis_1"),1.5f);S.ShieldCapacity=20.f;S.ShieldSecondsRemaining=4.f;
    TestTrue(TEXT("Valid data"),UAskirMagicComponent::ValidateSnapshotData(S,100.f));
    UAskirSaveGame* Save=NewObject<UAskirSaveGame>();Save->SavedMagic=S;
    TArray<uint8> Bytes;TestTrue(TEXT("Real SaveGame serializer"),UGameplayStatics::SaveGameToMemory(Save,Bytes));
    auto* Loaded=Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
    if(TestNotNull(TEXT("Deserialized save"),Loaded))
    {
        TestEqual(TEXT("Mana persisted"),Loaded->SavedMagic.Mana,65.f);
        TestEqual(TEXT("Shield persisted"),Loaded->SavedMagic.ShieldCapacity,20.f);
        TestEqual(TEXT("Cooldown duration persisted"),Loaded->SavedMagic.CooldownsRemaining[TEXT("eis_1")],1.5f);
    }
    S.SelectedSpellID=TEXT("unknown");TestFalse(TEXT("Unknown spell rejected"),UAskirMagicComponent::ValidateSnapshotData(S,100.f));
    S.SelectedSpellID=TEXT("eis_1");S.CooldownsRemaining[TEXT("eis_1")]=100.f;TestFalse(TEXT("Invalid cooldown rejected"),UAskirMagicComponent::ValidateSnapshotData(S,100.f));
    S.CooldownsRemaining.Reset();S.Mana=-1;TestFalse(TEXT("Negative mana rejected"),UAskirMagicComponent::ValidateSnapshotData(S,100.f));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirMagicTargetTest,"Askir.Magic.CivilianDormantAndDeadTargetProtection",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirMagicTargetTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;if(!TestNotNull(TEXT("World"),Fixture.World))return false;
    auto* Player=Fixture.Spawn<AAskirCharacter>();auto* Controller=Fixture.Spawn<APlayerController>();Controller->Possess(Player);
    auto* NPC=Fixture.Spawn<AAskirNPC>();NPC->bHostile=false;
    TestFalse(TEXT("Protect civilian"),UAskirMagicComponent::IsValidTarget(Player,NPC));
    NPC->bHostile=true;NPC->SetEncounterActive(false);TestFalse(TEXT("Protect dormant quest guardian"),UAskirMagicComponent::IsValidTarget(Player,NPC));
    NPC->SetEncounterActive(true);TestTrue(TEXT("Live enemy targetable"),UAskirMagicComponent::IsValidTarget(Player,NPC));
    NPC->CombatComponent->RestoreHealth(0.f);TestFalse(TEXT("Protect corpse"),UAskirMagicComponent::IsValidTarget(Player,NPC));
    return true;
}
#endif
