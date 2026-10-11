#include "Economy/AskirMerchantActor.h"
#include "Economy/AskirMerchantComponent.h"
#include "Components/StaticMeshComponent.h"
AAskirMerchantActor::AAskirMerchantActor()
{
 PrimaryActorTick.bCanEverTick=false;
 Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));RootComponent=Mesh;
 Merchant=CreateDefaultSubobject<UAskirMerchantComponent>(TEXT("Merchant"));
}
bool AAskirMerchantActor::Interact_Implementation(AActor* Interactor)
{
 if(!IsValid(Interactor)||InteractionRange<=0.f||FVector::DistSquared(GetActorLocation(),Interactor->GetActorLocation())>FMath::Square(InteractionRange))return false;
 OnMerchantOpened.Broadcast(Interactor);return true;
}
FText AAskirMerchantActor::GetInteractionText_Implementation()const
{
 return NSLOCTEXT("Askir","OpenMerchant","Handeln");
}
