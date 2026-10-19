#include "Interaction/AskirInteractionComponent.h"
#include "Interaction/AskirInteractable.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
UAskirInteractionComponent::UAskirInteractionComponent(){PrimaryComponentTick.bCanEverTick=false;}
bool UAskirInteractionComponent::TryInteract(float Distance)
{
    AActor*O=GetOwner();if(!O||!O->GetWorld())return false;FVector S=O->GetActorLocation()+FVector(0,0,50),E=S+O->GetActorForwardVector()*Distance;FHitResult H;FCollisionQueryParams P(SCENE_QUERY_STAT(AskirInteract),false,O);
    if(!O->GetWorld()->SweepSingleByChannel(H,S,E,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(TraceRadius),P))return false;
    AActor*T=H.GetActor();if(!T||!T->GetClass()->ImplementsInterface(UAskirInteractable::StaticClass()))return false;
    return IAskirInteractable::Execute_Interact(T,O);
}
