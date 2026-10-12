#include "Loot/AskirLootChest.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Quest/AskirQuestComponent.h"
#include "Mission/AskirDornfeldMissionComponent.h"
#include "Audio/AskirAudioLibrary.h"
#include "Components/StaticMeshComponent.h"
AAskirLootChest::AAskirLootChest()
{
 PrimaryActorTick.bCanEverTick=false;
 Visual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
 SetRootComponent(Visual);
 Visual->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}
bool AAskirLootChest::Interact_Implementation(AActor* Interactor)
{
 if(bInteractionInProgress || bOpened || !IsValid(Interactor) || !FMath::IsFinite(MaximumDistance) || MaximumDistance<=0.f || FVector::DistSquared(GetActorLocation(),Interactor->GetActorLocation())>FMath::Square(MaximumDistance))return false;
 UAskirDornfeldMissionComponent* Mission=Interactor->FindComponentByClass<UAskirDornfeldMissionComponent>();
 if(!DornfeldChestID.IsNone()&&(!Mission||!Mission->CanOpenMissionChest(DornfeldChestID)))return false;
 TGuardValue<bool> InteractionGuard(bInteractionInProgress,true);
 UAskirInventoryComponent* Inventory=Interactor->FindComponentByClass<UAskirInventoryComponent>();
 if(!Inventory)return false;
 for(const FAskirChestEntry& Entry:Contents)if(!IsValid(Entry.Item)||Entry.Quantity<=0)return false;
 TArray<FAskirInventoryEntry> Payload;
 for(const FAskirChestEntry& Entry:Contents)
 {
  FAskirInventoryEntry Item;Item.Item=Entry.Item;Item.Quantity=Entry.Quantity;
  Payload.Add(Item);
 }
 TArray<FAskirInventoryEntry> Prepared;
 if(!Inventory->PrepareExchange({},Payload,Prepared))return false;
 TGuardValue<bool> InventoryGuard(Inventory->bTransactionInProgress,true);
 if(!DornfeldChestID.IsNone()&&!Mission->OpenMissionChest(DornfeldChestID,false))return false;
 Inventory->Items=MoveTemp(Prepared);
 bOpened=true;
 if(!DornfeldChestID.IsNone())Mission->NotifyMissionRestored();
 if(DornfeldChestID.IsNone()&&!ObjectiveID.IsNone())
 {
  if(UAskirQuestComponent* Quest=Interactor->FindComponentByClass<UAskirQuestComponent>())
  {
   if(QuestID.IsNone())Quest->AdvanceObjective(ObjectiveID,1);
   else Quest->AdvanceQuestObjective(QuestID,ObjectiveID,1);
  }
 }
 UAskirAudioLibrary::PlayWorldCue(this,TEXT("Chest"));
 if(!Payload.IsEmpty())Inventory->OnInventoryChanged.Broadcast();
 OnOpened.Broadcast(Interactor);
 return true;
}
FText AAskirLootChest::GetInteractionText_Implementation()const
{
 return bOpened ? NSLOCTEXT("Askir","ChestEmpty","Truhe ist leer") : NSLOCTEXT("Askir","OpenChest","Truhe öffnen");
}

void AAskirLootChest::RestoreLootState(bool bConsumed)
{
 bOpened=bConsumed;
 SetActorHiddenInGame(bConsumed);
 SetActorEnableCollision(!bConsumed);
}
