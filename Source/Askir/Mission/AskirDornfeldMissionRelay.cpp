#include "Mission/AskirDornfeldMissionRelay.h"
#include "Mission/AskirDornfeldMissionComponent.h"
#include "Loot/AskirLootPickup.h"
#include "Loot/AskirLootChest.h"
#include "Combat/AskirCombatComponent.h"
#include "Characters/AskirNPC.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

AAskirDornfeldMissionRelay::AAskirDornfeldMissionRelay(){PrimaryActorTick.bCanEverTick=false;}
void AAskirDornfeldMissionRelay::BeginPlay()
{
 Super::BeginPlay();
 if(!BindMissionEvents() && GetWorld())
 {
  GetWorld()->GetTimerManager().SetTimer(BindRetryTimer,this,&AAskirDornfeldMissionRelay::RetryBindMissionEvents,FMath::Max(0.1f,BindRetryInterval),true);
 }
}
void AAskirDornfeldMissionRelay::RetryBindMissionEvents()
{
 if(BindMissionEvents() && GetWorld())GetWorld()->GetTimerManager().ClearTimer(BindRetryTimer);
}
bool AAskirDornfeldMissionRelay::BindMissionEvents()
{
 if(bBound)return true;
 if(EventID.IsNone())return false;
 if(!IsValid(MissionPlayer))MissionPlayer=UGameplayStatics::GetPlayerPawn(this,0);
 if(!IsValid(MissionPlayer))return false;
 BoundMission=MissionPlayer->FindComponentByClass<UAskirDornfeldMissionComponent>();
 if(!IsValid(BoundMission))return false;
 const int32 Sources=(IsValid(ShardPickup)?1:0)+(IsValid(MissionChest)?1:0)+(IsValid(GuardianActor)?1:0);
 if(Sources!=1){UE_LOG(LogTemp,Warning,TEXT("Dornfeld relay %s: configure exactly one source"),*GetName());return false;}
 if(IsValid(GuardianActor))
 {
  GuardianCombat=GuardianActor->FindComponentByClass<UAskirCombatComponent>();
  if(!IsValid(GuardianCombat))return false;
  GuardianCombat->OnDeath.AddUniqueDynamic(this,&AAskirDornfeldMissionRelay::OnGuardianDied);
 }
 if(IsValid(ShardPickup))
 {
  // Binding also installs the pre-consumption gate; generators should assign it in
  // the editor so that even an interaction before BeginPlay binding is gated.
  ShardPickup->DornfeldShardID=EventID;
  ShardPickup->OnCollected.AddUniqueDynamic(this,&AAskirDornfeldMissionRelay::OnShardCollected);
 }
 if(IsValid(MissionChest))
 {
  MissionChest->DornfeldChestID=EventID;
  MissionChest->OnOpened.AddUniqueDynamic(this,&AAskirDornfeldMissionRelay::OnChestOpened);
 }
 BoundMission->OnStageChanged.AddUniqueDynamic(this,&AAskirDornfeldMissionRelay::OnMissionStageChanged);
 bBound=true;
 RefreshMissionState();
 return true;
}
void AAskirDornfeldMissionRelay::RefreshMissionState()
{
 if(!IsValid(BoundMission))return;
 const FAskirDornfeldMissionSave& State=BoundMission->State;
 if(IsValid(ShardPickup))ShardPickup->RestoreLootState(State.ShardIDs.Contains(EventID));
 if(IsValid(MissionChest))MissionChest->RestoreLootState(State.ChestID==EventID);
 if(IsValid(GuardianActor)&&IsValid(GuardianCombat))
 {
  const bool bDefeated=State.GuardianIDs.Contains(EventID);
  const bool bActive=!bDefeated&&State.Stage==EAskirDornfeldStage::DefeatGuardians;
  if(AAskirNPC* NPC=Cast<AAskirNPC>(GuardianActor))
  {
   NPC->RestoreEncounterState(bDefeated);
   NPC->SetEncounterActive(bActive);
  }
  else
  {
   GuardianCombat->RestoreHealth(bDefeated?0.f:GuardianCombat->GetMaxHealth());
   GuardianCombat->SetDamageEnabled(bActive);
   GuardianActor->SetActorHiddenInGame(!bActive);
   GuardianActor->SetActorEnableCollision(bActive);
   GuardianActor->SetActorTickEnabled(bActive);
   GuardianActor->SetLifeSpan(0.f);
  }
 }
}
void AAskirDornfeldMissionRelay::OnMissionStageChanged(EAskirDornfeldStage Stage){RefreshMissionState();}
void AAskirDornfeldMissionRelay::OnShardCollected(AActor* Collector)
{
 if(Collector!=MissionPlayer||!IsValid(BoundMission))return;
 // New gated pickups already commit before their notification; this adapter is
 // harmless for them and preserves the old relay event contract.
 BoundMission->CollectShard(EventID);
}
void AAskirDornfeldMissionRelay::OnChestOpened(AActor* Opener)
{
 if(Opener!=MissionPlayer||!IsValid(BoundMission))return;
 BoundMission->OpenMissionChest(EventID);
}
void AAskirDornfeldMissionRelay::OnGuardianDied()
{
 if(!IsValid(BoundMission)||!IsValid(GuardianCombat)||!IsValid(GuardianActor))return;
 // Keep the placed guardian available for an earlier save to restore. Respawning
 // on an uncredited death avoids a permanently impossible mission objective.
 GuardianActor->SetLifeSpan(0.f);
 if(GuardianCombat->IsKillAttributedTo(MissionPlayer)&&BoundMission->DefeatGuardian(EventID))return;
 if(GetWorld())GetWorld()->GetTimerManager().SetTimerForNextTick(this,&AAskirDornfeldMissionRelay::RefreshMissionState);
}
void AAskirDornfeldMissionRelay::EndPlay(const EEndPlayReason::Type Reason)
{
 if(IsValid(ShardPickup))ShardPickup->OnCollected.RemoveDynamic(this,&AAskirDornfeldMissionRelay::OnShardCollected);
 if(IsValid(MissionChest))MissionChest->OnOpened.RemoveDynamic(this,&AAskirDornfeldMissionRelay::OnChestOpened);
 if(IsValid(GuardianCombat))GuardianCombat->OnDeath.RemoveDynamic(this,&AAskirDornfeldMissionRelay::OnGuardianDied);
 if(IsValid(BoundMission))BoundMission->OnStageChanged.RemoveDynamic(this,&AAskirDornfeldMissionRelay::OnMissionStageChanged);
 if(GetWorld())GetWorld()->GetTimerManager().ClearTimer(BindRetryTimer);
 bBound=false;Super::EndPlay(Reason);
}
