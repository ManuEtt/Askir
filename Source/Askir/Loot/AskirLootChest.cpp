#include "Loot/AskirLootChest.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Quest/AskirQuestComponent.h"
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
 if(bOpened || !IsValid(Interactor) || MaximumDistance<=0.f || FVector::DistSquared(GetActorLocation(),Interactor->GetActorLocation())>FMath::Square(MaximumDistance))return false;
 UAskirInventoryComponent* Inventory=Interactor->FindComponentByClass<UAskirInventoryComponent>();
 if(!Inventory)return false;
 for(const FAskirChestEntry& Entry:Contents)if(!IsValid(Entry.Item)||Entry.Quantity<=0)return false;
 TArray<FAskirChestEntry> Added;
 for(const FAskirChestEntry& Entry:Contents)
 {
  if(!Inventory->AddItem(Entry.Item,Entry.Quantity))
  {
   for(const FAskirChestEntry& Previous:Added)Inventory->RemoveItem(Previous.Item,Previous.Quantity);
   return false;
  }
  Added.Add(Entry);
 }
 bOpened=true;
 if(!ObjectiveID.IsNone())
 {
  if(UAskirQuestComponent* Quest=Interactor->FindComponentByClass<UAskirQuestComponent>())
  {
   if(QuestID.IsNone())Quest->AdvanceObjective(ObjectiveID,1);
   else Quest->AdvanceQuestObjective(QuestID,ObjectiveID,1);
  }
 }
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
