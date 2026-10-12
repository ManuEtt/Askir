#include "Mission/AskirDornfeldEventActor.h"
#include "Mission/AskirDornfeldMissionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Characters/AskirCharacter.h"
#include "Game/AskirPlayerController.h"
#include "Audio/AskirAudioLibrary.h"
AAskirDornfeldEventActor::AAskirDornfeldEventActor()
{
 PrimaryActorTick.bCanEverTick=false;
 Visual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
 SetRootComponent(Visual);
 Visual->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}
bool AAskirDornfeldEventActor::TriggerEvent(AActor* Player)
{
 if(!IsValid(Player)||!GetWorld()||!FMath::IsFinite(InteractionRange)||InteractionRange<=0.f||FVector::DistSquared(Player->GetActorLocation(),GetActorLocation())>FMath::Square(InteractionRange))return false;
 UAskirDornfeldMissionComponent* Mission=Player->FindComponentByClass<UAskirDornfeldMissionComponent>();
 if(!Mission)return false;
 switch(Event)
 {
 case EAskirDornfeldEvent::ElderDialogue:
 {
  const EAskirDornfeldStage Previous=Mission->GetStage();
  const bool bChanged=Mission->TalkToElder();
  if(bChanged&&Previous==EAskirDornfeldStage::NotStarted)UAskirAudioLibrary::PlayCue(this,TEXT("QuestAccepted"));
  else if(bChanged&&Previous==EAskirDornfeldStage::ReturnToElder)UAskirAudioLibrary::PlayCue(this,TEXT("QuestCompleted"));
  return bChanged;
 }
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
 if(Event==EAskirDornfeldEvent::ElderDialogue)
 {
  if(auto* Pawn=Cast<AAskirCharacter>(Interactor))
   if(auto* PC=Cast<AAskirPlayerController>(Pawn->GetController()))return PC->OpenElderDialogue(this);
  return false;
 }
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
