#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Save/AskirSaveGame.h"
#include "Save/AskirSaveSlotStore.h"
#include "World/AskirWorldClockComponent.h"
#include "Travel/AskirWorldTravelSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/StrongObjectPtr.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirSaveValidationTest,"Askir.Persistence.PayloadValidation",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirSaveValidationTest::RunTest(const FString& Parameters)
{
 TStrongObjectPtr<UAskirSaveGame> Save(NewObject<UAskirSaveGame>());
 FString Reason;
 TestTrue(TEXT("Default v3 snapshot validates"),Save->ValidateSnapshot(Reason));
 Save->PlayerHealth=std::numeric_limits<float>::quiet_NaN();
 TestFalse(TEXT("Nonfinite health rejects payload"),Save->ValidateSnapshot(Reason));
 Save->PlayerHealth=100.f;
 Save->SavedDornfeldMission.bRewardClaimed=true;
 TestFalse(TEXT("Unbacked mission reward is rejected"),Save->ValidateSnapshot(Reason));
 Save->SavedDornfeldMission.bRewardClaimed=false;
 Save->SaveFormatVersion=UAskirSaveGame::CurrentFormatVersion+1;
 TestFalse(TEXT("Future format rejected"),Save->ValidateSnapshot(Reason));
 TestFalse(TEXT("Path traversal slot rejected"),UAskirSaveGame::IsSafeSlotName(TEXT("../other")));
 TestTrue(TEXT("Ordinary slot accepted"),UAskirSaveGame::IsSafeSlotName(TEXT("AskirQuick_1")));
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirSaveMemoryTest,"Askir.Persistence.SystemSnapshotRoundTrip",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirSaveMemoryTest::RunTest(const FString& Parameters)
{
 TStrongObjectPtr<UAskirSaveGame> Save(NewObject<UAskirSaveGame>());
 Save->SavedSystems.Materials=27;Save->SavedSystems.Food=7.f;
 Save->SavedSystems.UnlockedRecipes.Add(FName(TEXT("Ration")));
 Save->SavedSystems.Needs.Hunger=33.f;
 Save->SavedWorldEventLastDays.Add(FName(TEXT("OneShot")),1);
 Save->SavedProgression.SkillRanks.Add(FName(TEXT("Sword")),3);
 TArray<uint8> Bytes;
 TestTrue(TEXT("Serialization succeeds"),UGameplayStatics::SaveGameToMemory(Save.Get(),Bytes));
 TStrongObjectPtr<UAskirSaveGame> Restored(Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes)));
 if(!TestNotNull(TEXT("Deserialize creates Askir snapshot"),Restored.Get()))return false;
 TestEqual(TEXT("Materials restored"),Restored->SavedSystems.Materials,27);
 TestEqual(TEXT("Food restored"),Restored->SavedSystems.Food,7.f);
 TestEqual(TEXT("Needs restored"),Restored->SavedSystems.Needs.Hunger,33.f);
 TestTrue(TEXT("Recipe restored"),Restored->SavedSystems.UnlockedRecipes.Contains(FName(TEXT("Ration"))));
 TestEqual(TEXT("Event ledger restored"),Restored->SavedWorldEventLastDays.FindRef(FName(TEXT("OneShot"))),1);
 TestEqual(TEXT("Nested progression restored"),Restored->SavedProgression.SkillRanks.FindRef(FName(TEXT("Sword"))),3);
 FString Reason;TestTrue(TEXT("Round trip remains valid"),Restored->ValidateSnapshot(Reason));
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirSaveBankTest,"Askir.Persistence.SaveBanksRecoverPreviousGeneration",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirSaveBankTest::RunTest(const FString& Parameters)
{
 const FString Slot=TEXT("AskirAutomation_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
 struct FSlotCleanup
 {
  FString Name;
  ~FSlotCleanup(){UGameplayStatics::DeleteGameInSlot(Name+TEXT("_BankA"),0);UGameplayStatics::DeleteGameInSlot(Name+TEXT("_BankB"),0);}
 } Cleanup{Slot};
 TStrongObjectPtr<UAskirSaveGame> Save(NewObject<UAskirSaveGame>());
 Save->SavedPlayerGold=90;
 if(!TestTrue(TEXT("First bank written and verified"),AskirSaveSlots::WriteNext(Slot,Save.Get())))return false;
 Save->SavedPlayerGold=170;
 if(!TestTrue(TEXT("Second generation written"),AskirSaveSlots::WriteNext(Slot,Save.Get())))return false;
 TStrongObjectPtr<UAskirSaveGame> Latest(AskirSaveSlots::ReadLatest(Slot));
 if(!TestNotNull(TEXT("Latest generation exists"),Latest.Get()))return false;
 TestEqual(TEXT("Newest valid generation selected"),Latest->SavedPlayerGold,170);
 Latest->SaveFormatVersion=999;
 TestTrue(TEXT("Create invalid newest bank"),UGameplayStatics::SaveGameToSlot(Latest.Get(),Slot+TEXT("_BankA"),0));
 AddExpectedError(TEXT("Askir rejected save bank"),EAutomationExpectedErrorFlags::Contains,1);
 TStrongObjectPtr<UAskirSaveGame> Recovered(AskirSaveSlots::ReadLatest(Slot));
 if(!TestNotNull(TEXT("Earlier valid generation recovers"),Recovered.Get()))return false;
 TestEqual(TEXT("Fallback preserves earlier gold"),Recovered->SavedPlayerGold,90);
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirClockRestoreTest,"Askir.Persistence.ClockRestoreDoesNotReplayEvents",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirClockRestoreTest::RunTest(const FString& Parameters)
{
 TStrongObjectPtr<UAskirWorldClockComponent> Clock(NewObject<UAskirWorldClockComponent>());
 FAskirTimedWorldEvent Event;Event.EventID=FName(TEXT("Arrival"));Event.Hour=12;Event.bRepeatDaily=false;
 Clock->TimedEvents.Add(Event);
 Clock->RestoreClock(1,12,{});
 TestEqual(TEXT("Restoring matching event hour does not trigger event"),Clock->TimedEvents[0].LastTriggeredDay,-1);
 TMap<FName,int32> Receipt;Receipt.Add(Event.EventID,1);
 Clock->RestoreClock(1,12,Receipt);
 Clock->AdvanceHours(24);
 TestEqual(TEXT("Completed one-shot cannot replay next day"),Clock->TimedEvents[0].LastTriggeredDay,1);
 Clock->RestoreClock(1,8,{});
 TestEqual(TEXT("Earlier save clears later event receipt"),Clock->TimedEvents[0].LastTriggeredDay,-1);
 return true;
}
#endif
