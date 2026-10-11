#include "Mission/AskirDornfeldEventActor.h"
#include "Mission/AskirDornfeldMissionComponent.h"
AAskirDornfeldEventActor::AAskirDornfeldEventActor(){PrimaryActorTick.bCanEverTick=false;}
bool AAskirDornfeldEventActor::TriggerEvent(AActor* Player)
{
 if(!IsValid(Player)||!GetWorld()||FVector::DistSquared(Player->GetActorLocation(),GetActorLocation())>FMath::Square(FMath::Max(0.f,InteractionRange)))return false;
 UAskirDornfeldMissionComponent* Mission=Player->FindComponentByClass<UAskirDornfeldMissionComponent>();
 if(!Mission)return false;
 switch(Event)
 {
 case EAskirDornfeldEvent::ElderDialogue:return Mission->TalkToElder();
 case EAskirDornfeldEvent::Shard:return Mission->CollectShard(PersistentEventID);
 case EAskirDornfeldEvent::GuardianDefeated:return false; // Require combat death relay.
 case EAskirDornfeldEvent::ChestOpened:return false; // Require actual loot chest open relay.
 }
 return false;
}
bool AAskirDornfeldEventActor::Interact_Implementation(AActor* Interactor)
{
 // Guardian death must be reported by the enemy's death event, not by interacting.
 if(Event==EAskirDornfeldEvent::GuardianDefeated||Event==EAskirDornfeldEvent::ChestOpened)return false;
 return TriggerEvent(Interactor);
}
FText AAskirDornfeldEventActor::GetInteractionText_Implementation() const
{
 switch(Event)
 {
 case EAskirDornfeldEvent::ElderDialogue:return NSLOCTEXT("AskirDornfeld","ElderInteract","Mit Mara sprechen");
 case EAskirDornfeldEvent::Shard:return NSLOCTEXT("AskirDornfeld","ShardInteract","Brunnenscherbe untersuchen");
 case EAskirDornfeldEvent::ChestOpened:return NSLOCTEXT("AskirDornfeld","ChestInteract","Truhe öffnen");
 default:return NSLOCTEXT("AskirDornfeld","GuardianInteract","Schattenwächter");
 }
}
