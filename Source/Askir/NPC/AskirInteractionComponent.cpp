#include "NPC/AskirInteractionComponent.h"
#include "GameFramework/Actor.h"
UAskirInteractionComponent::UAskirInteractionComponent(){PrimaryComponentTick.bCanEverTick=false;}
bool UAskirInteractionComponent::CanInteract(AActor* Interactor) const
{
 const AActor* Owner=GetOwner();
 return IsValid(Interactor)&&IsValid(Owner)&&Interactor!=Owner&&
 FVector::DistSquared(Interactor->GetActorLocation(),Owner->GetActorLocation())<=FMath::Square(FMath::Max(0.f,MaxInteractionDistance));
}
bool UAskirInteractionComponent::Interact(AActor* Interactor)
{
 if(!CanInteract(Interactor))return false;
 OnInteracted.Broadcast(Interactor);return true;
}
