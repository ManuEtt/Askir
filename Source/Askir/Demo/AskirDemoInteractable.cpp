#include "Demo/AskirDemoInteractable.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
AAskirDemoInteractable::AAskirDemoInteractable()
{
 PrimaryActorTick.bCanEverTick=false;
 Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));RootComponent=Mesh;
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 if(Cube.Succeeded())Mesh->SetStaticMesh(Cube.Object);
 Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
 Mesh->SetCollisionResponseToAllChannels(ECR_Block);
 Mesh->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
 Mesh->SetRelativeScale3D(FVector(.65f,.65f,1.4f));
}
void AAskirDemoInteractable::SetUsed(bool bNewUsed)
{
 bUsed=bNewUsed;SetActorHiddenInGame(bUsed);SetActorEnableCollision(!bUsed);
}
