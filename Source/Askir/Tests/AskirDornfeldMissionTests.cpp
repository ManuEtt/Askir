#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Mission/AskirDornfeldMissionComponent.h"
#include "Mission/AskirDornfeldMissionRelay.h"
#include "Loot/AskirLootPickup.h"
#include "Loot/AskirLootChest.h"
#include "Combat/AskirCombatComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Save/AskirSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace
{
struct FAskirMissionTestWorld : FAskirAutomationTestWorld
{
 AActor* Player=nullptr;
 UAskirDornfeldMissionComponent* Mission=nullptr;
 UAskirInventoryComponent* Bag=nullptr;
 UAskirQuestComponent* Journal=nullptr;
 UAskirTradeComponent* Wallet=nullptr;
 UAskirProgressionComponent* Progression=nullptr;
 FAskirMissionTestWorld()
 {
  if(!World)return;
  Player=Spawn<AActor>();
  if(!Player)return;
  Mission=AddComponent<UAskirDornfeldMissionComponent>(Player);
  Bag=AddComponent<UAskirInventoryComponent>(Player);
  Journal=AddComponent<UAskirQuestComponent>(Player);
  Wallet=AddComponent<UAskirTradeComponent>(Player);
  Progression=AddComponent<UAskirProgressionComponent>(Player);
 }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirDornfeldProgressionTest,"Askir.Mission.Dornfeld.ProgressionAndDedup",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirDornfeldProgressionTest::RunTest(const FString& Parameters)
{
 FAskirMissionTestWorld Fixture;
 if(!TestNotNull(TEXT("Spawned mission fixture"),Fixture.Mission))return false;
 UAskirDornfeldMissionComponent* Mission=Fixture.Mission;
 TestFalse(TEXT("Shard cannot be consumed before acceptance"),Mission->CollectShard(TEXT("A")));
 TestTrue(TEXT("Mara accepts mission"),Mission->TalkToElder());
 TestFalse(TEXT("Duplicate elder does not restart"),Mission->TalkToElder());
 TestFalse(TEXT("Chest cannot skip guardians"),Mission->OpenMissionChest(TEXT("Chest1")));
 TestFalse(TEXT("Empty shard ID"),Mission->CollectShard(NAME_None));
 TestTrue(TEXT("Shard A"),Mission->CollectShard(TEXT("A")));
 TestFalse(TEXT("Duplicate shard A"),Mission->CollectShard(TEXT("A")));
 TestTrue(TEXT("Shard B"),Mission->CollectShard(TEXT("B")));
 TestTrue(TEXT("Shard C"),Mission->CollectShard(TEXT("C")));
 TestEqual(TEXT("Guardian stage"),Mission->GetStage(),EAskirDornfeldStage::DefeatGuardians);
 TestTrue(TEXT("Guardian one"),Mission->DefeatGuardian(TEXT("G1")));
 TestFalse(TEXT("Duplicate guardian"),Mission->DefeatGuardian(TEXT("G1")));
 TestTrue(TEXT("Guardian two"),Mission->DefeatGuardian(TEXT("G2")));
 TestEqual(TEXT("Chest stage"),Mission->GetStage(),EAskirDornfeldStage::OpenChest);
 TestTrue(TEXT("Chest opened"),Mission->OpenMissionChest(TEXT("Chest1")));
 TestEqual(TEXT("Return stage"),Mission->GetStage(),EAskirDornfeldStage::ReturnToElder);
 const FAskirDornfeldMissionSave Snapshot=Mission->CaptureMission();
 Mission->RestoreMission(Snapshot);
 TestFalse(TEXT("No duplicate shard after restore"),Mission->CollectShard(TEXT("A")));
 TestFalse(TEXT("No duplicate chest after restore"),Mission->OpenMissionChest(TEXT("Chest1")));
 TestTrue(TEXT("Mara grants completion"),Mission->TalkToElder());
 TestEqual(TEXT("Exactly 90 gold"),Fixture.Wallet->Gold,90);
 TestEqual(TEXT("180 XP crosses the level-one threshold"),Fixture.Progression->Progress.Level,2);
 TestEqual(TEXT("80 XP remains after level up"),Fixture.Progression->Progress.Experience,80);
 TestFalse(TEXT("Mara cannot grant again"),Mission->TalkToElder());
 TestTrue(TEXT("Journal reward receipt recorded"),Fixture.Journal->HasClaimedReward(TEXT("DORNFELD_BRUNNEN")));
 UAskirSaveGame* Save=NewObject<UAskirSaveGame>();
 Save->SavedDornfeldMission=Mission->CaptureMission();
 TArray<uint8> Bytes;
 if(TestTrue(TEXT("Serialize mission via real SaveGame"),UGameplayStatics::SaveGameToMemory(Save,Bytes)))
 {
  UAskirSaveGame* Loaded=Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
  if(TestNotNull(TEXT("Load serialized mission"),Loaded))
  {
   Mission->RestoreMission(Loaded->SavedDornfeldMission);
   TestFalse(TEXT("Loaded reward receipt forbids duplicate reward"),Mission->TalkToElder());
   TestEqual(TEXT("Gold unchanged by repeated load"),Fixture.Wallet->Gold,90);
  }
 }
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirDornfeldRestoreValidationTest,"Askir.Mission.Dornfeld.RestoreEvidence",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirDornfeldRestoreValidationTest::RunTest(const FString& Parameters)
{
 FAskirMissionTestWorld Fixture;
 if(!TestNotNull(TEXT("Spawned mission fixture"),Fixture.Mission))return false;
 FAskirDornfeldMissionSave Corrupt;
 Corrupt.Stage=EAskirDornfeldStage::Completed;
 Corrupt.ShardIDs={NAME_None,FName(TEXT("A")),FName(TEXT("A")),FName(TEXT("B"))};
 Corrupt.GuardianIDs={FName(TEXT("G1")),FName(TEXT("G2"))};
 Corrupt.ChestID=TEXT("Cellar");
 Fixture.Mission->RestoreMission(Corrupt);
 TestEqual(TEXT("Missing unique shard evidence rewinds completion"),Fixture.Mission->GetStage(),EAskirDornfeldStage::CollectShards);
 TestEqual(TEXT("Deduplicated nonempty shard count"),Fixture.Mission->GetShardCount(),2);
 TestEqual(TEXT("Premature guardian receipts cleared"),Fixture.Mission->GetGuardianCount(),0);
 TestTrue(TEXT("Premature chest receipt cleared"),Fixture.Mission->State.ChestID.IsNone());
 TestFalse(TEXT("Corrupt stage cannot grant reward"),Fixture.Mission->TalkToElder());
 TestEqual(TEXT("No gold from corrupted snapshot"),Fixture.Wallet->Gold,0);
 Corrupt.bRewardClaimed=true;
 Fixture.Mission->RestoreMission(Corrupt);
 TestEqual(TEXT("Existing reward receipt remains complete"),Fixture.Mission->GetStage(),EAskirDornfeldStage::Completed);
 TestFalse(TEXT("Reward receipt never pays during restore"),Fixture.Mission->TalkToElder());
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirDornfeldNativeEventTest,"Askir.Mission.Dornfeld.NativeLootCombatEvents",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirDornfeldNativeEventTest::RunTest(const FString& Parameters)
{
 FAskirMissionTestWorld Fixture;
 if(!TestNotNull(TEXT("Spawned mission fixture"),Fixture.Mission))return false;
 UAskirItemData* Shard=NewObject<UAskirItemData>();
 Shard->ItemID=TEXT("WELL_SHARD");Shard->MaxStackSize=3;
 AAskirLootChest* Chest=Fixture.World->SpawnActor<AAskirLootChest>();
 Chest->DornfeldChestID=TEXT("CELLAR_CHEST");
 FAskirChestEntry ChestItem;ChestItem.Item=Shard;ChestItem.Quantity=1;Chest->Contents.Add(ChestItem);
 AAskirLootPickup* Pickups[3];
 const FName ShardIDs[3]={TEXT("SHARD_NORTH"),TEXT("SHARD_EAST"),TEXT("SHARD_WELL")};
 for(int32 Index=0;Index<3;++Index)
 {
  Pickups[Index]=Fixture.World->SpawnActor<AAskirLootPickup>();
  Pickups[Index]->Item=Shard;Pickups[Index]->DornfeldShardID=ShardIDs[Index];
 }
 TestFalse(TEXT("Early chest does not transfer loot"),Chest->Interact_Implementation(Fixture.Player));
 TestFalse(TEXT("Early shard does not transfer loot"),Pickups[0]->Interact_Implementation(Fixture.Player));
 TestEqual(TEXT("Bag stays empty before acceptance"),Fixture.Bag->GetItemCount(Shard),0);
 AActor* Guardians[2];UAskirCombatComponent* Combat[2];AAskirDornfeldMissionRelay* Relays[2];
 for(int32 Index=0;Index<2;++Index)
 {
  Guardians[Index]=Fixture.World->SpawnActor<AActor>();
  Combat[Index]=Fixture.AddComponent<UAskirCombatComponent>(Guardians[Index]);
  Relays[Index]=Fixture.World->SpawnActor<AAskirDornfeldMissionRelay>();
  Relays[Index]->MissionPlayer=Fixture.Player;Relays[Index]->GuardianActor=Guardians[Index];
  Relays[Index]->EventID=Index==0?FName(TEXT("GUARDIAN_1")):FName(TEXT("GUARDIAN_2"));
  TestTrue(TEXT("Bind real guardian death delegate"),Relays[Index]->BindMissionEvents());
 }
 FAskirDamageInfo Damage;Damage.Damage=1000.f;Damage.DamageCauser=Fixture.Player;
 TestEqual(TEXT("Guardians cannot die before shard stage"),Combat[0]->ApplyDamage(Damage),0.f);
 Fixture.Mission->TalkToElder();
 for(AAskirLootPickup* Pickup:Pickups)TestTrue(TEXT("Actual shard item transfer advances mission"),Pickup->Interact_Implementation(Fixture.Player));
 TestFalse(TEXT("Actual pickup cannot be farmed"),Pickups[0]->Interact_Implementation(Fixture.Player));
 TestEqual(TEXT("Three physical shards entered bag"),Fixture.Bag->GetItemCount(Shard),3);
 Combat[0]->ApplyDamage(Damage);
 TestEqual(TEXT("Attributed death adds one guardian receipt"),Fixture.Mission->GetGuardianCount(),1);
 FAskirDamageInfo Environmental=Damage;Environmental.DamageCauser=nullptr;
 Combat[1]->ApplyDamage(Environmental);
 TestEqual(TEXT("Environmental death cannot count for player"),Fixture.Mission->GetGuardianCount(),1);
 Relays[1]->RefreshMissionState();
 TestFalse(TEXT("Uncredited guardian remains available"),Combat[1]->IsDead());
 Combat[1]->ApplyDamage(Damage);
 TestEqual(TEXT("Two distinct credited guardians"),Fixture.Mission->GetGuardianCount(),2);
 TestTrue(TEXT("Real chest transfer advances return stage"),Chest->Interact_Implementation(Fixture.Player));
 TestFalse(TEXT("Chest cannot grant twice"),Chest->Interact_Implementation(Fixture.Player));
 TestTrue(TEXT("Mara grants 90 gold and 180 XP"),Fixture.Mission->TalkToElder());
 TestEqual(TEXT("Native chain completion"),Fixture.Mission->GetStage(),EAskirDornfeldStage::Completed);
 return true;
}
#endif
