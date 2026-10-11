#include "Loot/AskirLootPickup.h"
#include "Inventory/AskirItemData.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
AAskirLootPickup::AAskirLootPickup()
{
 PrimaryActorTick.bCanEverTick=false;
 InteractionSphere=CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
 SetRootComponent(InteractionSphere);
 InteractionSphere->InitSphereRadius(65.f);
 InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
 Visual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
 Visual->SetupAttachment(InteractionSphere);
 Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
bool AAskirLootPickup::Interact_Implementation(AActor* Interactor)
{
 if(bCollected || !IsValid(Interactor) || !IsValid(Item) || Quantity<=0 || MaximumDistance<=0.f)return false;
 if(FVector::DistSquared(Interactor->GetActorLocation(),GetActorLocation())>FMath::Square(MaximumDistance))return false;
 UAskirInventoryComponent* Inventory=Interactor->FindComponentByClass<UAskirInventoryComponent>();
 if(!Inventory || !Inventory->AddItem(Item,Quantity))return false;
 bCollected=true;
 if(!ObjectiveID.IsNone())
 {
  if(UAskirQuestComponent* Quest=Interactor->FindComponentByClass<UAskirQuestComponent>())
  {
   if(QuestID.IsNone())Quest->AdvanceObjective(ObjectiveID,Quantity);
   else Quest->AdvanceQuestObjective(QuestID,ObjectiveID,Quantity);
  }
 }
 OnCollected.Broadcast(Interactor);
 SetActorEnableCollision(false);
 SetActorHiddenInGame(true);
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
