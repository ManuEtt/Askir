#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Combat/AskirCombatComponent.h"
#include "Mission/AskirDornfeldMissionComponent.h"
#include "Mission/AskirDornfeldMissionRelay.h"
#include "Mission/AskirDornfeldEventActor.h"
#include "Loot/AskirLootPickup.h"
#include "Loot/AskirLootChest.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Progression/AskirProgressionComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Game/AskirGameMode.h"
#include "Save/AskirSaveGame.h"
#include "Interaction/AskirInteractable.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Package.h"

namespace
{
int64 TotalEarnedExperience(const FAskirProgressionSnapshot& Progress)
{
    int64 Total=Progress.Experience;
    for(int64 Level=1;Level<Progress.Level;++Level)Total+=100*Level*Level;
    return Total;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirDornfeldPlayerIntegrationTest,"Askir.Mission.Dornfeld.SpawnedPlayerCombatLootSaveAndMara",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirDornfeldPlayerIntegrationTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Initialized runtime world"),Fixture.World))return false;
    AActor* Floor=Fixture.Spawn<AActor>();
    UBoxComponent* FloorCollision=Fixture.AddComponent<UBoxComponent>(Floor);
    Floor->SetRootComponent(FloorCollision);
    FloorCollision->SetBoxExtent(FVector(600.f,600.f,20.f));
    FloorCollision->SetCollisionProfileName(TEXT("BlockAll"));
    Floor->SetActorLocation(FVector(0.f,0.f,-20.f));
    AAskirCharacter* Player=Fixture.Spawn<AAskirCharacter>(FVector(0.f,0.f,96.f));
    APlayerController* Controller=Fixture.Spawn<APlayerController>();
    Controller->Possess(Player);
    Controller->SetControlRotation(FRotator::ZeroRotator);
    if(!TestTrue(TEXT("Possessed character is the actual quest player"),Player->IsPlayerControlled()))return false;
    if(!TestEqual(TEXT("GameplayStatics resolves possessed player"),UGameplayStatics::GetPlayerPawn(Fixture.World,0),static_cast<APawn*>(Player)))return false;
    AAskirGameMode* Mode=Fixture.Spawn<AAskirGameMode>();
    Player->BaseAttackDamage=200.f; // One-hit fixture avoids testing balance in an integration test.
    Player->AttackCooldown=0.f;
    Player->TradeComponent->Gold=0;
    Player->ProgressionComponent->RestoreProgress(FAskirProgressionSnapshot());
    UAskirDornfeldMissionComponent* Mission=Player->DornfeldMission;

    // A resident /Game test asset exercises soft-reference save/load. No disk asset is claimed.
    const FString PackageName=TEXT("/Game/Askir/Automation/")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    UPackage* ItemPackage=CreatePackage(*PackageName);
    UAskirItemData* Shard=NewObject<UAskirItemData>(ItemPackage,TEXT("Brunnenscherbe"),RF_Public);
    Shard->ItemID=TEXT("WELL_SHARD");Shard->MaxStackSize=3;
    AAskirDornfeldEventActor* Mara=Fixture.Spawn<AAskirDornfeldEventActor>(FVector(0.f,-150.f,96.f));
    Mara->Event=EAskirDornfeldEvent::ElderDialogue;
    Mara->PersistentEventID=TEXT("MARA");
    AAskirLootPickup* Pickups[3];
    const FName ShardIDs[3]={TEXT("SHARD_NORTH"),TEXT("SHARD_EAST"),TEXT("SHARD_WELL")};
    for(int32 Index=0;Index<3;++Index)
    {
        Pickups[Index]=Fixture.Spawn<AAskirLootPickup>(FVector(0.f,150.f+Index*10.f,96.f));
        Pickups[Index]->Item=Shard;Pickups[Index]->Quantity=1;
        Pickups[Index]->DornfeldShardID=ShardIDs[Index];Pickups[Index]->PersistentLootID=ShardIDs[Index];
    }
    AAskirLootChest* Chest=Fixture.Spawn<AAskirLootChest>(FVector(0.f,200.f,96.f));
    Chest->DornfeldChestID=TEXT("CELLAR_CHEST");Chest->PersistentLootID=TEXT("CELLAR_CHEST");
    AAskirNPC* Guardians[2];
    AAskirDornfeldMissionRelay* Relays[2];
    for(int32 Index=0;Index<2;++Index)
    {
        Guardians[Index]=Fixture.Spawn<AAskirNPC>(FVector(Index==0?120.f:-120.f,0.f,96.f));
        Guardians[Index]->DefeatObjectiveID=TEXT("Kill_ShadowGuardian");
        Relays[Index]=Fixture.Spawn<AAskirDornfeldMissionRelay>();
        Relays[Index]->MissionPlayer=Player;Relays[Index]->GuardianActor=Guardians[Index];
        Relays[Index]->EventID=Index==0?FName(TEXT("GUARDIAN_1")):FName(TEXT("GUARDIAN_2"));
        TestTrue(TEXT("Native guardian death relay binds"),Relays[Index]->BindMissionEvents());
    }
    TestFalse(TEXT("Dormant guardians cannot be attacked"),Guardians[0]->CombatComponent->IsDamageEnabled());
    TestFalse(TEXT("Shard cannot bypass Mara"),IAskirInteractable::Execute_Interact(Pickups[0],Player));
    TestTrue(TEXT("Confirmed Mara dialogue accepts the mission"),Mara->TriggerEvent(Player));
    TestFalse(TEXT("Chest cannot bypass the fight"),IAskirInteractable::Execute_Interact(Chest,Player));
    for(AAskirLootPickup* Pickup:Pickups)TestTrue(TEXT("Physical shard pickup succeeds"),IAskirInteractable::Execute_Interact(Pickup,Player));
    TestFalse(TEXT("One shard cannot be collected twice"),IAskirInteractable::Execute_Interact(Pickups[0],Player));
    TestEqual(TEXT("Three unique shards in journal"),Mission->GetShardCount(),3);
    TestEqual(TEXT("Three physical shard items in inventory"),Player->InventoryComponent->GetItemCount(Shard),3);
    TestTrue(TEXT("Guardian stage activates the native enemy"),Guardians[0]->IsEncounterActive());

    Player->Attack();
    TestTrue(TEXT("Attack schedules a delayed hit"),Player->HasPendingAttack());
    TestFalse(TEXT("No death at button press"),Guardians[0]->IsDead());
    Fixture.World->Tick(LEVELTICK_All,.25f);
    if(!TestTrue(TEXT("Timer impact sweeps and kills first guardian"),Guardians[0]->IsDead()))return false;
    TestEqual(TEXT("One credited guardian death"),Mission->GetGuardianCount(),1);
    UAskirSaveGame* Halfway=Mode->CaptureSnapshot();
    if(!TestNotNull(TEXT("Full player/loot/quest snapshot after first kill"),Halfway))return false;
    TArray<uint8> HalfwayBytes;
    if(!TestTrue(TEXT("Serialize actual mid-mission SaveGame"),UGameplayStatics::SaveGameToMemory(Halfway,HalfwayBytes)))return false;

    Controller->SetControlRotation(FRotator(0.f,180.f,0.f));
    Player->Attack();Fixture.World->Tick(LEVELTICK_All,.25f);
    if(!TestTrue(TEXT("Second native combat death"),Guardians[1]->IsDead()))return false;
    TestEqual(TEXT("Both guardian identities recorded"),Mission->GetGuardianCount(),2);
    TestTrue(TEXT("Actual cellar chest opens"),IAskirInteractable::Execute_Interact(Chest,Player));
    TestFalse(TEXT("Cellar chest cannot open twice"),IAskirInteractable::Execute_Interact(Chest,Player));
    const int64 XPBeforeTurnIn=TotalEarnedExperience(Player->ProgressionComponent->Progress);
    TestTrue(TEXT("Confirmed return dialogue finishes mission"),Mara->TriggerEvent(Player));
    TestEqual(TEXT("Mara grants exactly 90 gold"),Player->TradeComponent->Gold,90);
    TestEqual(TEXT("Mara grants exactly 180 XP in addition to enemy XP"),TotalEarnedExperience(Player->ProgressionComponent->Progress)-XPBeforeTurnIn,int64(180));
    TestTrue(TEXT("Quest journal stores claimed reward"),Player->QuestComponent->HasClaimedReward(TEXT("DORNFELD_BRUNNEN")));

    UAskirSaveGame* LoadedHalfway=Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromMemory(HalfwayBytes));
    if(!TestNotNull(TEXT("Deserialize actual mid-mission SaveGame"),LoadedHalfway))return false;
    if(!TestTrue(TEXT("Restore the complete captured snapshot"),Mode->RestoreSnapshot(LoadedHalfway,false)))return false;
    TestEqual(TEXT("Restore rewinds mission to one guardian"),Mission->GetGuardianCount(),1);
    TestEqual(TEXT("Restore rewinds unclaimed reward gold"),Player->TradeComponent->Gold,0);
    TestTrue(TEXT("Saved defeated guardian stays defeated"),Guardians[0]->IsDead());
    TestFalse(TEXT("Unconsumed guardian revives on earlier load"),Guardians[1]->IsDead());
    TestFalse(TEXT("Cellar chest becomes unopened on earlier load"),Chest->IsOpened());
    TestEqual(TEXT("Restored inventory preserves three shards"),Player->InventoryComponent->GetItemCount(Shard),3);
    TestFalse(TEXT("Loaded shard cannot be farmed"),IAskirInteractable::Execute_Interact(Pickups[0],Player));
    Controller->SetControlRotation(FRotator(0.f,180.f,0.f));
    Player->Attack();Fixture.World->Tick(LEVELTICK_All,.25f);
    if(!TestTrue(TEXT("Restored guardian can be defeated normally"),Guardians[1]->IsDead()))return false;
    TestTrue(TEXT("Restored chest opens normally"),IAskirInteractable::Execute_Interact(Chest,Player));
    TestTrue(TEXT("Restored mission can finish normally"),Mara->TriggerEvent(Player));
    UAskirSaveGame* Completed=Mode->CaptureSnapshot();
    if(!TestNotNull(TEXT("Completed snapshot"),Completed))return false;
    const int64 CompletedXP=TotalEarnedExperience(Player->ProgressionComponent->Progress);
    TestTrue(TEXT("Repeated full completed restore succeeds"),Mode->RestoreSnapshot(Completed,false));
    TestFalse(TEXT("Completed save forbids duplicate Mara reward"),Mara->TriggerEvent(Player));
    TestEqual(TEXT("Repeated load preserves gold"),Player->TradeComponent->Gold,90);
    TestEqual(TEXT("Repeated load preserves XP"),TotalEarnedExperience(Player->ProgressionComponent->Progress),CompletedXP);
    return true;
}
#endif
