#include "Mission/AskirDornfeldMissionRelay.h"
#include "Mission/AskirDornfeldMissionComponent.h"
#include "Loot/AskirLootPickup.h"
#include "Loot/AskirLootChest.h"
#include "Combat/AskirCombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
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
 if(bBound||EventID.IsNone())return false;
 if(!IsValid(MissionPlayer))MissionPlayer=UGameplayStatics::GetPlayerPawn(this,0);
 if(!IsValid(MissionPlayer)||!MissionPlayer->FindComponentByClass<UAskirDornfeldMissionComponent>())return false;
 const int32 Sources=(IsValid(ShardPickup)?1:0)+(IsValid(MissionChest)?1:0)+(IsValid(GuardianActor)?1:0);
 if(Sources!=1){UE_LOG(LogTemp,Warning,TEXT("Dornfeld relay %s: configure exactly one source"),*GetName());return false;}
 if(IsValid(GuardianActor))
 {
  GuardianCombat=GuardianActor->FindComponentByClass<UAskirCombatComponent>();
  if(!IsValid(GuardianCombat))return false;
  GuardianCombat->OnDeath.AddUniqueDynamic(this,&AAskirDornfeldMissionRelay::OnGuardianDied);
 }
 if(IsValid(ShardPickup))ShardPickup->OnCollected.AddUniqueDynamic(this,&AAskirDornfeldMissionRelay::OnShardCollected);
 if(IsValid(MissionChest))MissionChest->OnOpened.AddUniqueDynamic(this,&AAskirDornfeldMissionRelay::OnChestOpened);
 bBound=true;return true;
}
void AAskirDornfeldMissionRelay::OnShardCollected(AActor* Collector)
{
 if(Collector!=MissionPlayer)return;
 if(UAskirDornfeldMissionComponent* Mission=MissionPlayer->FindComponentByClass<UAskirDornfeldMissionComponent>())Mission->CollectShard(EventID);
}
void AAskirDornfeldMissionRelay::OnChestOpened(AActor* Opener)
{
 if(Opener!=MissionPlayer)return;
 if(UAskirDornfeldMissionComponent* Mission=MissionPlayer->FindComponentByClass<UAskirDornfeldMissionComponent>())Mission->OpenMissionChest(EventID);
}
void AAskirDornfeldMissionRelay::OnGuardianDied()
{
 if(!IsValid(MissionPlayer)||!IsValid(GuardianCombat))return;
 // A guardian killed by another NPC, hazards or another player must not advance this player's mission.
 AActor* Killer=GuardianCombat->GetLastDamageCauser();
 if(!IsValid(Killer))return;
 const bool bPlayerKill=Killer==MissionPlayer || Killer->GetOwner()==MissionPlayer || Killer->GetInstigator()==Cast<APawn>(MissionPlayer);
 if(!bPlayerKill)return;
 if(UAskirDornfeldMissionComponent* Mission=MissionPlayer->FindComponentByClass<UAskirDornfeldMissionComponent>())Mission->DefeatGuardian(EventID);
}
void AAskirDornfeldMissionRelay::EndPlay(const EEndPlayReason::Type Reason)
{
 if(IsValid(ShardPickup))ShardPickup->OnCollected.RemoveDynamic(this,&AAskirDornfeldMissionRelay::OnShardCollected);
 if(IsValid(MissionChest))MissionChest->OnOpened.RemoveDynamic(this,&AAskirDornfeldMissionRelay::OnChestOpened);
 if(IsValid(GuardianCombat))GuardianCombat->OnDeath.RemoveDynamic(this,&AAskirDornfeldMissionRelay::OnGuardianDied);
 if(GetWorld())GetWorld()->GetTimerManager().ClearTimer(BindRetryTimer);
 bBound=false;Super::EndPlay(Reason);
}
