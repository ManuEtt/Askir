#include "Travel/AskirWorldPortal.h"
#include "Travel/AskirWorldTravelSubsystem.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Game/AskirGameMode.h"
#include "Misc/PackageName.h"
#include "Engine/World.h"

AAskirWorldPortal::AAskirWorldPortal()
{
 PrimaryActorTick.bCanEverTick=false;
 Trigger=CreateDefaultSubobject<UBoxComponent>(TEXT("TravelTrigger"));
 SetRootComponent(Trigger);
 Trigger->SetBoxExtent(FVector(120.f,220.f,220.f));
 Trigger->SetCollisionProfileName(TEXT("Trigger"));
 Trigger->OnComponentBeginOverlap.AddDynamic(this,&AAskirWorldPortal::OnOverlap);
}
void AAskirWorldPortal::OnOverlap(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,int32,bool,const FHitResult&)
{
 Travel(Other);
}
bool AAskirWorldPortal::Travel(AActor* Traveler)
{
 if(!bEnabled || bTransitioning || DestinationMap.IsNone() || !Traveler || !GetWorld())return false;
 APawn* Pawn=Cast<APawn>(Traveler);
 if(!Pawn || !Cast<APlayerController>(Pawn->GetController()))return false;
 const FName CurrentMap(*UGameplayStatics::GetCurrentLevelName(this,true));
 if(CurrentMap==DestinationMap)return false;
 // A packaged build must contain the destination world. Never overwrite the
 // travel snapshot when the destination package is absent.
 const FString DestinationPackage = FString::Printf(TEXT("/Game/Maps/%s"), *DestinationMap.ToString());
 if (!FPackageName::DoesPackageExist(DestinationPackage))
 {
  UE_LOG(LogTemp, Warning, TEXT("Askir travel refused: destination map missing: %s"), *DestinationPackage);
  return false;
 }
 UGameInstance* Instance=GetGameInstance();
 UAskirWorldTravelSubsystem* State=Instance?Instance->GetSubsystem<UAskirWorldTravelSubsystem>():nullptr;
 if(!State)return false;
 bTransitioning=true;
 // Refuse travel if player state could not be safely persisted.
 AAskirGameMode* Mode=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this));
 if(!Mode || !Mode->SaveGame(TEXT("AskirTravelSnapshot")))
 {
  bTransitioning=false;
  return false;
 }
 State->bRestoreTravelSnapshot=true;
 State->QueueArrival(DestinationMap,CurrentMap);
 UGameplayStatics::OpenLevel(this,DestinationMap);
 return true;
}
