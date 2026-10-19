#include "Crafting/AskirCraftingStationActor.h"
#include "Crafting/AskirCraftingComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Characters/AskirCharacter.h"
#include "Game/AskirPlayerController.h"
#include "Game/AskirGameMode.h"
#include "Kismet/GameplayStatics.h"

AAskirCraftingStationActor::AAskirCraftingStationActor()
{
    PrimaryActorTick.bCanEverTick=false;
    StationName=NSLOCTEXT("Askir","Workbench","Werkbank");
    Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent=Mesh;
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
}
bool AAskirCraftingStationActor::CanInteract(AActor* Interactor) const
{
    const auto* Player=Cast<AAskirCharacter>(Interactor);
    if(!IsValid(Player)||Player->IsDead()||!Player->CraftingComponent||
       Player->CraftingComponent->IsTransactionInProgress()||!FMath::IsFinite(InteractionRange)||
       InteractionRange<=0.f||GetActorLocation().ContainsNaN()||Player->GetActorLocation().ContainsNaN())return false;
    if(const auto* GM=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))if(GM->IsPersistenceBusy())return false;
    return FVector::DistSquared(GetActorLocation(),Player->GetActorLocation())<=FMath::Square(InteractionRange);
}
bool AAskirCraftingStationActor::Interact_Implementation(AActor* Interactor)
{
    auto* Player=Cast<AAskirCharacter>(Interactor);
    if(!CanInteract(Player)||!Player->CanUseGameplayInput())return false;
    auto* PC=Cast<AAskirPlayerController>(Player->GetController());
    return PC&&PC->OpenCraftingStation(this);
}
FText AAskirCraftingStationActor::GetInteractionText_Implementation() const
{
    return NSLOCTEXT("Askir","Craft","Herstellen");
}
