#include "NPC/AskirNPCInteractionComponent.h"
#include "GameFramework/Actor.h"
UAskirNPCInteractionComponent::UAskirNPCInteractionComponent(){PrimaryComponentTick.bCanEverTick=false;}
bool UAskirNPCInteractionComponent::CanInteract(AActor* Interactor) const
{
 const AActor* Owner=GetOwner();
 return FMath::IsFinite(MaxInteractionDistance)&&IsValid(Interactor)&&IsValid(Owner)&&Interactor!=Owner&&
 FVector::DistSquared(Interactor->GetActorLocation(),Owner->GetActorLocation())<=FMath::Square(FMath::Max(0.f,MaxInteractionDistance));
}
bool UAskirNPCInteractionComponent::Interact(AActor* Interactor)
{
 if(!CanInteract(Interactor))return false;
 OnInteracted.Broadcast(Interactor);return true;
}
