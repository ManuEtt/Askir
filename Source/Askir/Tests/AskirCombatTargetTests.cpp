#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Combat/AskirCombatComponent.h"
#include "Combat/AskirCombatTargetPolicy.h"
#include "GameFramework/PlayerController.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirCombatTargetPolicyTest,"Askir.Combat.TargetPolicyProtectsCiviliansAndInactiveEncounters",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirCombatTargetPolicyTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Runtime world"),Fixture.World))return false;
    auto* Player=Fixture.Spawn<AAskirCharacter>();
    auto* OtherPlayer=Fixture.Spawn<AAskirCharacter>();
    auto* Enemy=Fixture.Spawn<AAskirNPC>();
    auto* Civilian=Fixture.Spawn<AAskirNPC>();
    Fixture.Spawn<APlayerController>()->Possess(Player);
    Fixture.Spawn<APlayerController>()->Possess(OtherPlayer);
    Enemy->bHostile=true;
    using AskirCombatTargetPolicy::CanDamage;
    TestTrue(TEXT("Player attacks an active hostile NPC"),CanDamage(Player,Enemy));
    TestTrue(TEXT("Active hostile NPC attacks a player"),CanDamage(Enemy,Player));
    TestFalse(TEXT("Player cannot damage civilian"),CanDamage(Player,Civilian));
    TestFalse(TEXT("Civilian cannot initiate ordinary combat"),CanDamage(Civilian,Player));
    TestFalse(TEXT("Hostile NPC cannot damage civilian"),CanDamage(Enemy,Civilian));
    TestFalse(TEXT("Player cannot damage another player"),CanDamage(Player,OtherPlayer));
    TestFalse(TEXT("Self damage is excluded from ordinary attacks"),CanDamage(Player,Player));
    TestFalse(TEXT("Missing source is excluded"),CanDamage(nullptr,Enemy));
    TestFalse(TEXT("Missing target is excluded"),CanDamage(Player,nullptr));
    Enemy->SetEncounterActive(false);
    TestFalse(TEXT("Dormant enemy cannot be attacked"),CanDamage(Player,Enemy));
    TestFalse(TEXT("Dormant enemy cannot attack"),CanDamage(Enemy,Player));
    Enemy->SetEncounterActive(true);
    Enemy->CombatComponent->SetDamageEnabled(false);
    TestFalse(TEXT("Damage-disabled source cannot attack"),CanDamage(Enemy,Player));
    TestFalse(TEXT("Damage-disabled target cannot be attacked"),CanDamage(Player,Enemy));
    Enemy->CombatComponent->SetDamageEnabled(true);
    Enemy->CombatComponent->RestoreHealth(0.f);
    TestFalse(TEXT("Dead source cannot attack"),CanDamage(Enemy,Player));
    TestFalse(TEXT("Dead target cannot be attacked"),CanDamage(Player,Enemy));
    Enemy->RestoreEncounterState(false);
    FAskirAutomationTestWorld OtherWorld;
    if(!TestNotNull(TEXT("Second runtime world"),OtherWorld.World))return false;
    auto* ForeignEnemy=OtherWorld.Spawn<AAskirNPC>();ForeignEnemy->bHostile=true;
    TestFalse(TEXT("Cross-world target cannot be attacked"),CanDamage(Player,ForeignEnemy));
    Civilian->CombatComponent->ApplyDamage(FAskirDamageInfo());
    FAskirDamageInfo EnvironmentalHit;EnvironmentalHit.Damage=10.f;
    const float PreviousHealth=Civilian->CombatComponent->GetHealth();
    TestTrue(TEXT("Explicit low-level environmental damage remains supported"),Civilian->CombatComponent->ApplyDamage(EnvironmentalHit)>0.f);
    TestTrue(TEXT("Explicit environmental hit changes civilian health"),Civilian->CombatComponent->GetHealth()<PreviousHealth);
    Enemy->Destroy();
    TestFalse(TEXT("Destroyed target is excluded"),CanDamage(Player,Enemy));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirFrontalGuardPolicyTest,"Askir.Combat.FrontalGuardResolvesDirectionsAndProjectileSources",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirFrontalGuardPolicyTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Runtime world"),Fixture.World))return false;
    auto* Defender=Fixture.Spawn<AAskirCharacter>();
    auto* Source=Fixture.Spawn<AAskirCharacter>(FVector(200,0,0));
    auto* Projectile=Fixture.Spawn<AActor>();Projectile->SetInstigator(Source);
    Defender->SetActorRotation(FRotator::ZeroRotator);
    FAskirDamageInfo Damage;Damage.Damage=10.f;Damage.HitDirection=FVector(-1,0,0);
    using AskirCombatTargetPolicy::IsFrontalBlock;
    TestTrue(TEXT("Front hit points toward defender and can be guarded"),IsFrontalBlock(Defender,Damage));
    Damage.HitDirection=FVector(1,0,0);
    TestFalse(TEXT("Rear hit bypasses directional guard"),IsFrontalBlock(Defender,Damage));
    Damage.HitDirection=FVector::ZeroVector;
    TestFalse(TEXT("Unknown direction and source never grant guard"),IsFrontalBlock(Defender,Damage));
    Damage.DamageCauser=Source;
    TestTrue(TEXT("Missing direction uses a valid front causer"),IsFrontalBlock(Defender,Damage));
    Source->SetActorLocation(FVector(-200,0,0));
    TestFalse(TEXT("Rear causer is not frontal"),IsFrontalBlock(Defender,Damage));
    Source->SetActorLocation(FVector(200,0,0));Damage.DamageCauser=Projectile;
    TestTrue(TEXT("Coincident projectile resolves its front instigator"),IsFrontalBlock(Defender,Damage));
    Damage.HitDirection=FVector(1,0,0);
    TestFalse(TEXT("Reliable projectile direction takes priority over instigator location"),IsFrontalBlock(Defender,Damage));
    Damage.HitDirection=FVector(-1,0,0);Damage.DamageType=EAskirDamageType::TrueDamage;
    TestFalse(TEXT("True damage bypasses frontal guard"),IsFrontalBlock(Defender,Damage));
    Damage.DamageType=EAskirDamageType::Physical;
    TestFalse(TEXT("NaN angle is rejected"),IsFrontalBlock(Defender,Damage,std::numeric_limits<float>::quiet_NaN()));
    TestFalse(TEXT("Wide rear-reaching angle is rejected"),IsFrontalBlock(Defender,Damage,180.f));
    Damage.HitDirection.X=std::numeric_limits<double>::quiet_NaN();
    TestFalse(TEXT("Malformed direction cannot use a good causer fallback"),IsFrontalBlock(Defender,Damage));
    Damage.HitDirection=FVector::ZeroVector;Projectile->SetInstigator(nullptr);Projectile->SetOwner(Source);
    TestTrue(TEXT("Coincident projectile can resolve its owner"),IsFrontalBlock(Defender,Damage));
    Source->SetActorLocation(Defender->GetActorLocation());
    TestFalse(TEXT("Coincident source chain has no reliable bearing"),IsFrontalBlock(Defender,Damage));
    TestFalse(TEXT("Missing defender is rejected"),IsFrontalBlock(nullptr,Damage));
    FAskirAutomationTestWorld OtherWorld;
    if(!TestNotNull(TEXT("Second runtime world"),OtherWorld.World))return false;
    Damage.DamageCauser=OtherWorld.Spawn<AActor>(FVector(200,0,0));
    TestFalse(TEXT("Cross-world source cannot provide guard direction"),IsFrontalBlock(Defender,Damage));
    return true;
}
#endif
