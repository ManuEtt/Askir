#include "Economy/AskirMerchantActor.h"
#include "Economy/AskirMerchantComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Characters/AskirCharacter.h"
#include "Game/AskirPlayerController.h"
#include "Game/AskirGameMode.h"
#include "Economy/AskirVillageEconomyCatalog.h"
#include "Kismet/GameplayStatics.h"
AAskirMerchantActor::AAskirMerchantActor()
{
 PrimaryActorTick.bCanEverTick=false;
 MerchantName=NSLOCTEXT("Askir","Merchant","Händler");
 Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));RootComponent=Mesh;
 Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
 Mesh->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
 Merchant=CreateDefaultSubobject<UAskirMerchantComponent>(TEXT("Merchant"));
}
void AAskirMerchantActor::BeginPlay()
{
 AskirVillageEconomyCatalog::ConfigureMerchant(Merchant);
 Super::BeginPlay();
}
bool AAskirMerchantActor::CanInteract(AActor* Interactor) const
{
 const auto* Player=Cast<AAskirCharacter>(Interactor);
 if(!IsValid(Player)||Player->IsDead()||!IsValid(Merchant)||Merchant->IsTransactionInProgress()||
    !FMath::IsFinite(InteractionRange)||InteractionRange<=0.f||
    GetActorLocation().ContainsNaN()||Player->GetActorLocation().ContainsNaN())return false;
 if(const auto* GM=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))if(GM->IsPersistenceBusy())return false;
 return FVector::DistSquared(GetActorLocation(),Player->GetActorLocation())<=FMath::Square(InteractionRange);
}
bool AAskirMerchantActor::Interact_Implementation(AActor* Interactor)
{
 auto* Player=Cast<AAskirCharacter>(Interactor);
 if(!CanInteract(Player)||!Player->CanUseGameplayInput())return false;
 auto* PC=Cast<AAskirPlayerController>(Player->GetController());
 if(!PC||!PC->OpenMerchant(this))return false;
 OnMerchantOpened.Broadcast(Interactor);return true;
}
FText AAskirMerchantActor::GetInteractionText_Implementation()const
{
 return NSLOCTEXT("Askir","OpenMerchant","Handeln");
}
