#include "World/AskirPickupActor.h"
#include "Components/StaticMeshComponent.h"
#include "Inventory/AskirItemData.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "UObject/ConstructorHelpers.h"
AAskirPickupActor::AAskirPickupActor(){PrimaryActorTick.bCanEverTick=false;Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));RootComponent=Mesh;static ConstructorHelpers::FObjectFinder<UStaticMesh>A(TEXT("/Engine/BasicShapes/Sphere.Sphere"));if(A.Succeeded())Mesh->SetStaticMesh(A.Object);}
bool AAskirPickupActor::Interact_Implementation(AActor*I){if(!IsValid(ItemData)||!IsValid(I)||Quantity<=0)return false;UAskirInventoryComponent*Inv=I->FindComponentByClass<UAskirInventoryComponent>();if(!Inv||!Inv->AddItem(ItemData,Quantity))return false;if(UAskirQuestComponent* Quests=I->FindComponentByClass<UAskirQuestComponent>()){if(!ItemData->ItemID.IsNone())Quests->AdvanceObjective(ItemData->ItemID,Quantity);}Destroy();return true;}
FText AAskirPickupActor::GetInteractionText_Implementation()const{return ItemData?FText::Format(NSLOCTEXT("Askir","Pickup","Aufheben: {0}"),ItemData->DisplayName):NSLOCTEXT("Askir","PickupUnknown","Aufheben");}