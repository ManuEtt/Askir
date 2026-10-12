#include "World/AskirEncounterSpawner.h"
#include "World/AskirEncounterZone.h"
#include "Combat/AskirCombatComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/Crc.h"
AAskirEncounterSpawner::AAskirEncounterSpawner()
{
 PrimaryActorTick.bCanEverTick=false;
 SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}
void AAskirEncounterSpawner::BeginPlay()
{
 Super::BeginPlay();
 if(IsValid(Zone))Zone->OnEncounterTriggered.AddDynamic(this,&AAskirEncounterSpawner::HandleEncounterTriggered);
}
void AAskirEncounterSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
 if(IsValid(Zone))Zone->OnEncounterTriggered.RemoveDynamic(this,&AAskirEncounterSpawner::HandleEncounterTriggered);
 bRunning=false;ActivePlayer=nullptr;LivingEnemies.Reset();
 Super::EndPlay(EndPlayReason);
}
void AAskirEncounterSpawner::HandleEncounterTriggered(AActor* Player,FName EncounterID)
{
 StartForPlayer(Player,EncounterID);
}
bool AAskirEncounterSpawner::StartForPlayer(AActor* Player,FName EncounterID)
{
 const APawn* Pawn=Cast<APawn>(Player);
 if(bRunning||bFinished||!IsValid(Pawn)||!Pawn->IsPlayerControlled()||!EnemyClass||!GetWorld())return false;
 // Reject external requests for a different zone; never credit the wrong encounter.
 if(IsValid(Zone) && !Zone->EncounterID.IsNone() && EncounterID!=Zone->EncounterID)return false;
 // A completed persistent encounter cannot be restarted by revisiting the zone.
 if(IsValid(Zone) && !Zone->EncounterID.IsNone())
  if(const UAskirQuestComponent* Quests=Player->FindComponentByClass<UAskirQuestComponent>())
   if(Quests->HasCompletedEncounter(Zone->EncounterID))return false;
 // Persistent encounters require a quest ledger before spawning; otherwise
 // their rewards could be granted without any durable completion marker.
 if(IsValid(Zone) && !Zone->EncounterID.IsNone() &&
    !IsValid(Player->FindComponentByClass<UAskirQuestComponent>()))return false;
 ActivePlayer=Player;
 WavesStarted=0;
 bRunning=true;
 if(!SpawnWave()) {bRunning=false;ActivePlayer=nullptr;return false;}
 return true;
}
bool AAskirEncounterSpawner::SpawnWave()
{
 UWorld* World=GetWorld();
 if(!World||!EnemyClass||WavesStarted>=FMath::Clamp(WaveCount,1,10))return false;
 LivingEnemies.Reset();
 const int32 Count=FMath::Clamp(EnemiesPerWave,1,32);
 for(int32 Index=0;Index<Count;++Index)
 {
  const float Angle=(2.f*PI*Index)/Count;
  const FVector Position=GetActorLocation()+FVector(FMath::Cos(Angle),FMath::Sin(Angle),0.f)*FMath::Max(0.f,SpawnRadius);
  FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
  APawn* Enemy=World->SpawnActor<APawn>(EnemyClass,Position,GetActorRotation(),Params);
  if(!IsValid(Enemy))continue;
  Enemy->SpawnDefaultController();
  UAskirCombatComponent* Combat=Enemy->FindComponentByClass<UAskirCombatComponent>();
  if(!IsValid(Combat)) {Enemy->Destroy();continue;}
  UAskirEncounterEnemyTracker* Tracker=NewObject<UAskirEncounterEnemyTracker>(Enemy);
  Tracker->Spawner=this;
  Tracker->Combat=Combat;
  Enemy->AddInstanceComponent(Tracker);
  Tracker->RegisterComponent();
  Combat->OnDeath.AddDynamic(Tracker,&UAskirEncounterEnemyTracker::HandleDeath);
  LivingEnemies.Add(Enemy);
 }
 if(LivingEnemies.IsEmpty())return false;
 ++WavesStarted;
 return true;
}
void UAskirEncounterEnemyTracker::OnComponentDestroyed(bool bDestroyingHierarchy)
{
 if(IsValid(Combat))Combat->OnDeath.RemoveDynamic(this,&UAskirEncounterEnemyTracker::HandleDeath);
 Super::OnComponentDestroyed(bDestroyingHierarchy);
}
void UAskirEncounterEnemyTracker::HandleDeath()
{
 if(IsValid(Combat))Combat->OnDeath.RemoveDynamic(this,&UAskirEncounterEnemyTracker::HandleDeath);
 if(IsValid(Spawner))Spawner->NotifyEnemyDefeated();
}
void AAskirEncounterSpawner::NotifyEnemyDefeated()
{
 if(!bRunning||bTransitioningWave)return;
 TGuardValue<bool> TransitionGuard(bTransitioningWave,true);
 for(int32 Index=LivingEnemies.Num()-1;Index>=0;--Index)
 {
  APawn* Enemy=LivingEnemies[Index].Get();
  const UAskirCombatComponent* Combat=IsValid(Enemy)?Enemy->FindComponentByClass<UAskirCombatComponent>():nullptr;
  if(!Combat||Combat->IsDead())LivingEnemies.RemoveAtSwap(Index);
 }
 if(!LivingEnemies.IsEmpty())return;
 if(WavesStarted<FMath::Clamp(WaveCount,1,10))
 {
  if(SpawnWave())return;
  // A failed spawn is not a completed encounter: allow an explicit retry.
  bRunning=false;ActivePlayer=nullptr;LivingEnemies.Reset();
  return;
 }
 FinishEncounter();
}
void AAskirEncounterSpawner::FinishEncounter()
{
 if(!bRunning||bFinished)return;
 bRunning=false;bFinished=true;
 if(IsValid(ActivePlayer))
 {
  if(UAskirQuestComponent* Quests=ActivePlayer->FindComponentByClass<UAskirQuestComponent>())
  {
   // If a reward is configured, ensure a recipient inventory exists before
   // recording permanent completion; otherwise a reward could be lost forever.
   UAskirInventoryComponent* Inventory=ActivePlayer->FindComponentByClass<UAskirInventoryComponent>();
   // Select at most one valid weighted entry; empty/disabled tables use the legacy reward.
   UAskirItemData* SelectedReward=CompletionRewardItem;
   int32 SelectedQuantity=FMath::Clamp(CompletionRewardQuantity,1,999);
   int64 TotalWeight=0;
   for(const FAskirEncounterLootEntry& Entry:CompletionLootTable)
    if(IsValid(Entry.Item) && Entry.Weight>0) TotalWeight+=FMath::Clamp(Entry.Weight,0,100000);
   if(TotalWeight>0)
   {
    // Stable encounter IDs give the same drop on retries and after reload.
    // Repeatable encounters (no ID) still draw a fresh reward on each victory.
    const FName RewardEncounterID=IsValid(Zone)?Zone->EncounterID:NAME_None;
    const uint32 Seed=RewardEncounterID.IsNone()
     ? static_cast<uint32>(FMath::Rand())
     : FCrc::StrCrc32(*RewardEncounterID.ToString());
    FRandomStream RewardRandom(static_cast<int32>(Seed));
    const int64 Draw=FMath::Min(TotalWeight-1,
     static_cast<int64>(RewardRandom.FRand()*static_cast<double>(TotalWeight)));
    int64 Cursor=0;
    for(const FAskirEncounterLootEntry& Entry:CompletionLootTable)
    {
     if(!IsValid(Entry.Item) || Entry.Weight<=0)continue;
     Cursor+=FMath::Clamp(Entry.Weight,0,100000);
     if(Draw<Cursor){SelectedReward=Entry.Item;SelectedQuantity=FMath::Clamp(Entry.Quantity,1,999);break;}
    }
   }
   const bool bHasReward=IsValid(SelectedReward);
   if(bHasReward && !IsValid(Inventory))
   {
    // Keep the encounter retryable if the player cannot receive the reward.
    bFinished=false;
    ActivePlayer=nullptr;
    return;
   }
   const FName ID=IsValid(Zone)?Zone->EncounterID:NAME_None;
   if(!ID.IsNone() && Quests->HasCompletedEncounter(ID))
   {
    ActivePlayer=nullptr;
    return;
   }
   // Never mark a reward as complete if inventory insertion failed.
   // Crash-atomic persistence still requires a future save transaction journal.
   if(bHasReward && !Inventory->AddItem(SelectedReward,SelectedQuantity))
   {
    bFinished=false;ActivePlayer=nullptr;
    return;
   }
   const bool bNewCompletion=ID.IsNone() || Quests->MarkEncounterCompleted(ID);
   // Combat zones must not also grant the objective on entry.
   const bool bZoneAwardsOnEntry=IsValid(Zone) && Zone->bAwardObjectiveOnEntry;
   const FName Quest=CompletionQuestID.IsNone() && IsValid(Zone)?Zone->QuestID:CompletionQuestID;
   const FName Objective=CompletionObjectiveID.IsNone() && IsValid(Zone)?Zone->ObjectiveID:CompletionObjectiveID;
   if(bNewCompletion && !bZoneAwardsOnEntry && !Quest.IsNone() && !Objective.IsNone())
    Quests->AdvanceQuestObjective(Quest,Objective,IsValid(Zone)?FMath::Max(1,Zone->ObjectiveAmount):1);
  }
 }
 OnEncounterFinished.Broadcast(ActivePlayer.Get(),IsValid(Zone)?Zone->EncounterID:NAME_None);
 ActivePlayer=nullptr;
}
void AAskirEncounterSpawner::ResetSpawner()
{
 // Does not destroy already spawned enemies; avoid resetting during active combat.
 if(bRunning)return;
 bTransitioningWave=false;
 bFinished=false;WavesStarted=0;ActivePlayer=nullptr;LivingEnemies.Reset();
}
