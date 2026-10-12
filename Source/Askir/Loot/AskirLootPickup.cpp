#include "Loot/AskirLootPickup.h"
#include "Inventory/AskirItemData.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Mission/AskirDornfeldMissionComponent.h"
#include "Audio/AskirAudioLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
AAskirLootPickup::AAskirLootPickup()
{
 PrimaryActorTick.bCanEverTick=false;
 InteractionSphere=CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
 SetRootComponent(InteractionSphere);
 InteractionSphere->InitSphereRadius(65.f);
 InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
 InteractionSphere->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
 Visual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
 Visual->SetupAttachment(InteractionSphere);
 Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
bool AAskirLootPickup::Interact_Implementation(AActor* Interactor)
{
 if(bInteractionInProgress || bCollected || !IsValid(Interactor) || !IsValid(Item) || Quantity<=0 || !FMath::IsFinite(MaximumDistance) || MaximumDistance<=0.f)return false;
 if(FVector::DistSquared(Interactor->GetActorLocation(),GetActorLocation())>FMath::Square(MaximumDistance))return false;
 UAskirDornfeldMissionComponent* Mission=Interactor->FindComponentByClass<UAskirDornfeldMissionComponent>();
 if(!DornfeldShardID.IsNone()&&(!Mission||!Mission->CanCollectShard(DornfeldShardID)))return false;
 TGuardValue<bool> InteractionGuard(bInteractionInProgress,true);
 UAskirInventoryComponent* Inventory=Interactor->FindComponentByClass<UAskirInventoryComponent>();
 if(!Inventory)return false;
 FAskirInventoryEntry Added;Added.Item=Item;Added.Quantity=Quantity;
 TArray<FAskirInventoryEntry> Prepared;
 if(!Inventory->PrepareExchange({},{Added},Prepared))return false;
 TGuardValue<bool> InventoryGuard(Inventory->bTransactionInProgress,true);
 // No callbacks between the inventory, mission and persistent source commits.
 if(!DornfeldShardID.IsNone()&&!Mission->CollectShard(DornfeldShardID,false))return false;
 Inventory->Items=MoveTemp(Prepared);
 bCollected=true;
 SetActorEnableCollision(false);
 SetActorHiddenInGame(true);
 if(!DornfeldShardID.IsNone())Mission->NotifyMissionRestored();
 if(DornfeldShardID.IsNone()&&!ObjectiveID.IsNone())
 {
  if(UAskirQuestComponent* Quest=Interactor->FindComponentByClass<UAskirQuestComponent>())
  {
   if(QuestID.IsNone())Quest->AdvanceObjective(ObjectiveID,Quantity);
   else Quest->AdvanceQuestObjective(QuestID,ObjectiveID,Quantity);
  }
 }
 UAskirAudioLibrary::PlayWorldCue(this,TEXT("Shard"));
 Inventory->OnInventoryChanged.Broadcast();
 OnCollected.Broadcast(Interactor);
 // Keep actor in world for save/load snapshots; it is hidden and non-interactive.
 return true;
}
FText AAskirLootPickup::GetInteractionText_Implementation()const
{
 return !bCollected && IsValid(Item) ? FText::Format(NSLOCTEXT("Askir","PickupItem","Aufheben: {0}"),Item->DisplayName) : FText::GetEmpty();
}

void AAskirLootPickup::RestoreLootState(bool bConsumed)
{
 bCollected=bConsumed;
 SetActorHiddenInGame(bConsumed);
 SetActorEnableCollision(!bConsumed);
}
