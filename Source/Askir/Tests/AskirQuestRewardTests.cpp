#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Tests/AskirTransactionPersistenceProbe.h"
#include "Characters/AskirCharacter.h"
#include "Quest/AskirQuestComponent.h"
#include "Quest/AskirQuestData.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Progression/AskirProgressionComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "Game/AskirGameMode.h"
#include "GameFramework/PlayerController.h"

namespace
{
 UAskirQuestData* RewardQuest(FName ID)
 {
  auto* Q=NewObject<UAskirQuestData>();Q->QuestID=ID;Q->GoldReward=90;Q->ExperienceReward=180;
  FAskirQuestObjective Objective;Objective.ObjectiveID=TEXT("DELIVER");Q->Objectives.Add(Objective);
  return Q;
 }
 bool Ready(UAskirQuestComponent* Journal,UAskirQuestData* Quest)
 {return Journal->StartQuest(Quest) && Journal->AdvanceQuestObjective(Quest->QuestID,TEXT("DELIVER"));}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirQuestRewardAtomicFailureTest,"Askir.Quest.Rewards.RejectInvalidProgressAndCapacityAtomically",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirQuestRewardAtomicFailureTest::RunTest(const FString& Parameters)
{
 FAskirAutomationTestWorld Fixture;
 if(!TestNotNull(TEXT("Initialized runtime world"),Fixture.World))return false;
 auto* Player=Fixture.Spawn<AAskirCharacter>();
 if(!TestNotNull(TEXT("Spawned reward recipient"),Player))return false;
 auto* Q=RewardQuest(TEXT("REWARD_ATOMIC"));
 auto* Reward=NewObject<UAskirItemData>();Reward->ItemID=TEXT("REWARD_ITEM");Reward->MaxStackSize=1;
 Q->RewardItem=Reward;Q->RewardItemQuantity=2;Player->InventoryComponent->MaxInventorySlots=1;
 if(!TestTrue(TEXT("Quest objectives fulfilled"),Ready(Player->QuestComponent,Q)))return false;
 TestFalse(TEXT("Insufficient reward capacity rejects turn-in"),Player->QuestComponent->TurnInQuestWithRewards(Q));
 TestEqual(TEXT("Capacity rejection does not credit gold"),Player->TradeComponent->Gold,0);
 TestEqual(TEXT("Capacity rejection does not credit XP"),Player->ProgressionComponent->Progress.Experience,0);
 TestFalse(TEXT("Capacity rejection leaves reward unclaimed"),Player->QuestComponent->HasClaimedReward(Q->QuestID));
 TestEqual(TEXT("Capacity rejection preserves active quest"),Player->QuestComponent->GetState(Q),EAskirQuestState::Active);
 Player->InventoryComponent->MaxInventorySlots=2;
 Player->ProgressionComponent->Progress.Experience=-1;
 TestFalse(TEXT("Negative existing XP cannot consume the receipt"),Player->QuestComponent->TurnInQuestWithRewards(Q));
 TestEqual(TEXT("Invalid XP leaves inventory unchanged"),Player->InventoryComponent->Items.Num(),0);
 Player->ProgressionComponent->Progress.Experience=0;Player->ProgressionComponent->Progress.UnspentPoints=MAX_int32;
 TestFalse(TEXT("An earned point cannot silently saturate"),Player->QuestComponent->TurnInQuestWithRewards(Q));
 TestEqual(TEXT("Point overflow leaves gold unchanged"),Player->TradeComponent->Gold,0);
 Player->ProgressionComponent->Progress.UnspentPoints=0;
 Player->ProgressionComponent->Progress.Level=100;Player->ProgressionComponent->Progress.Experience=MAX_int32;
 TestFalse(TEXT("XP overflow rejects the complete reward"),Player->QuestComponent->TurnInQuestWithRewards(Q));
 Player->ProgressionComponent->Progress=FAskirProgressionSnapshot();
 auto& Objective=Player->QuestComponent->ObjectivesByQuest[Q->QuestID].Objectives[0];Objective.CurrentCount=2;
 TestFalse(TEXT("Corrupt over-counted objectives cannot authorize reward"),Player->QuestComponent->TurnInQuestWithRewards(Q));
 Objective.CurrentCount=1;
 TestTrue(TEXT("Corrected valid turn-in succeeds"),Player->QuestComponent->TurnInQuestWithRewards(Q));
 TestEqual(TEXT("Complete gold reward"),Player->TradeComponent->Gold,90);
 TestEqual(TEXT("XP level gained"),Player->ProgressionComponent->Progress.Level,2);
 TestEqual(TEXT("Complete XP remainder"),Player->ProgressionComponent->Progress.Experience,80);
 TestEqual(TEXT("Complete reward item quantity"),Player->InventoryComponent->GetItemCount(Reward),2);
 TestFalse(TEXT("Reward receipt prevents duplicate turn-in"),Player->QuestComponent->TurnInQuestWithRewards(Q));
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirQuestRewardPersistenceGateTest,"Askir.Persistence.QuestRewardDelegateCannotCaptureOrRewind",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirQuestRewardPersistenceGateTest::RunTest(const FString& Parameters)
{
 FAskirAutomationTestWorld Fixture;
 if(!TestNotNull(TEXT("Initialized runtime world"),Fixture.World))return false;
 auto* Player=Fixture.Spawn<AAskirCharacter>();auto* Controller=Fixture.Spawn<APlayerController>();
 auto* Mode=Fixture.Spawn<AAskirGameMode>();
 if(!TestNotNull(TEXT("Reward recipient"),Player) || !TestNotNull(TEXT("Player controller"),Controller) || !TestNotNull(TEXT("Save mode"),Mode))return false;
 Controller->Possess(Player);
 auto* Q=RewardQuest(TEXT("REWARD_REENTRANT"));
 if(!TestTrue(TEXT("Reward quest ready"),Ready(Player->QuestComponent,Q)))return false;
 auto* Before=Mode->CaptureSnapshot();
 if(!TestNotNull(TEXT("Snapshot before turn-in"),Before))return false;
 auto* Probe=NewObject<UAskirTransactionPersistenceProbe>();Probe->Mode=Mode;Probe->Snapshot=Before;
 // This award has no inventory reward: only the quest guard can block a save
 // from the real progression delegate, after wallet + XP + receipt commit.
 Player->ProgressionComponent->OnProgressionChanged.AddDynamic(Probe,&UAskirTransactionPersistenceProbe::ObserveInventoryChange);
 TestTrue(TEXT("Turn-in with real progression delegate"),Player->QuestComponent->TurnInQuestWithRewards(Q));
 Player->ProgressionComponent->OnProgressionChanged.RemoveDynamic(Probe,&UAskirTransactionPersistenceProbe::ObserveInventoryChange);
 TestTrue(TEXT("Progression delegate observed"),Probe->bCallbackObserved);
 TestTrue(TEXT("No partial snapshot from quest callback"),Probe->bCaptureBlocked);
 TestTrue(TEXT("Callback cannot rewind its own receipt"),Probe->bRestoreBlocked);
 TestEqual(TEXT("Final wallet survives rejected callback restore"),Player->TradeComponent->Gold,90);
 TestTrue(TEXT("Final quest receipt survives"),Player->QuestComponent->HasClaimedReward(Q->QuestID));
 TestFalse(TEXT("Quest transaction closes after callbacks"),Player->QuestComponent->IsTransactionInProgress());
 TestNotNull(TEXT("Save allowed after final notification"),Mode->CaptureSnapshot());
 return true;
}
#endif
