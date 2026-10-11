#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Mission/AskirDornfeldMissionComponent.h"
#include "GameFramework/Actor.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirDornfeldProgressionTest,"Askir.Mission.Dornfeld.ProgressionAndDedup",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirDornfeldProgressionTest::RunTest(const FString& Parameters)
{
 AActor* Owner=NewObject<AActor>(GetTransientPackage());
 UAskirDornfeldMissionComponent* Mission=NewObject<UAskirDornfeldMissionComponent>(Owner);
 TestTrue(TEXT("Elder starts mission"),Mission->TalkToElder());
 TestFalse(TEXT("Duplicate elder does not restart"),Mission->TalkToElder());
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
 UAskirDornfeldMissionComponent* Restored=NewObject<UAskirDornfeldMissionComponent>(Owner);
 Restored->RestoreMission(Snapshot);
 TestEqual(TEXT("Restored stage"),Restored->GetStage(),EAskirDornfeldStage::ReturnToElder);
 TestEqual(TEXT("Restored shards"),Restored->GetShardCount(),3);
 TestEqual(TEXT("Restored guardians"),Restored->GetGuardianCount(),2);
 TestFalse(TEXT("No duplicate shard after load"),Restored->CollectShard(TEXT("A")));
 TestFalse(TEXT("No duplicate chest after load"),Restored->OpenMissionChest(TEXT("Chest1")));
 return true;
}
#endif
