#include "Travel/AskirWorldPortal.h"
#include "Travel/AskirWorldTravelSubsystem.h"
#include "Travel/AskirTravelRules.h"
#include "Characters/AskirCharacter.h"
#include "Quest/AskirQuestComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Game/AskirGameMode.h"
#include "Misc/PackageName.h"
#include "Engine/World.h"

namespace
{
AskirTravelRules::Point Point(const FVector& V){return {V.X,V.Y,V.Z};}
FString DestinationFor(const AAskirWorldPortal* Portal)
{
 return Portal->DestinationPackagePath.IsEmpty()?FString::Printf(TEXT("/Game/Askir/WorldV42/%s"),*Portal->DestinationMap.ToString()):Portal->DestinationPackagePath;
}
}
AAskirWorldPortal::AAskirWorldPortal()
{
 PrimaryActorTick.bCanEverTick=false;
 Trigger=CreateDefaultSubobject<UBoxComponent>(TEXT("TravelTrigger"));
 SetRootComponent(Trigger);
 Trigger->SetBoxExtent(FVector(120.f,220.f,220.f));
 // Query only: visibility sweeps discover the E interaction, but walking is never blocked.
 Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
 Trigger->SetCollisionObjectType(ECC_WorldDynamic);
 Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
 Trigger->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
 Trigger->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
 Trigger->SetGenerateOverlapEvents(true);
 Trigger->OnComponentBeginOverlap.AddDynamic(this,&AAskirWorldPortal::OnOverlap);
 InteractionLabel=NSLOCTEXT("Askir","TravelRoute","Gebiet betreten");
}
void AAskirWorldPortal::OnOverlap(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,int32,bool,const FHitResult&)
{
 if(bTravelOnOverlap)Travel(Other);
}
bool AAskirWorldPortal::CanTravel(AActor* Traveler) const
{
 auto* Player=Cast<AAskirCharacter>(Traveler);
 if(!bEnabled||bTransitioning||DestinationMap.IsNone()||!GetWorld()||!Player||Player->GetWorld()!=GetWorld()
  ||UGameplayStatics::GetPlayerPawn(this,0)!=Player||!Cast<APlayerController>(Player->GetController())
  ||!Player->CanUseGameplayInput())return false;
 if(!AskirTravelRules::InRange(Point(Player->GetActorLocation()),Point(GetActorLocation()),InteractionRange))return false;
 if(!RequiredQuestID.IsNone())
 {
  if(!Player->QuestComponent)return false;
  const EAskirQuestState State=Player->QuestComponent->GetStateByID(RequiredQuestID);
  if(bRequireQuestCompleted?State!=EAskirQuestState::Completed:(State!=EAskirQuestState::Active&&State!=EAskirQuestState::Completed))return false;
 }
 const FName CurrentMap(*UGameplayStatics::GetCurrentLevelName(this,true));
 if(CurrentMap==DestinationMap)return false;
 const auto* Instance=GetGameInstance();
 const auto* State=Instance?Instance->GetSubsystem<UAskirWorldTravelSubsystem>():nullptr;
 if(!State||State->HasPendingTravel()||GetWorld()->GetTimeSeconds()<State->ArrivalGraceUntilSeconds)return false;
 return UAskirWorldTravelSubsystem::IsUsableWorldPackage(DestinationFor(this),DestinationMap);
}
bool AAskirWorldPortal::Travel(AActor* Traveler)
{
 if(!CanTravel(Traveler))return false;
 auto* State=GetGameInstance()->GetSubsystem<UAskirWorldTravelSubsystem>();
 auto* Mode=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this));
 if(!State||!Mode||Mode->IsPersistenceBusy())return false;
 // Guard before capture: a delegate cannot queue a competing overlap snapshot.
 TGuardValue<bool> TransitionGuard(bTransitioning,true);
 UAskirSaveGame* Snapshot=Mode->CaptureSnapshot();
 const FName CurrentMap(*UGameplayStatics::GetCurrentLevelName(this,true));
 if(!Snapshot||!State->QueueSnapshot(DestinationMap,CurrentMap,Snapshot,false))return false;
 State->PendingArrivalPortalID=DestinationPortalID;
 State->PendingDeparturePortalID=PortalID;
 if(auto* Player=Cast<AAskirCharacter>(Traveler))
  Player->SuspendForWorldTravel();
 // The GI-owned snapshot is authoritative until arrival succeeds; no disk writes.
 UGameplayStatics::OpenLevel(this,FName(*DestinationFor(this)));
 return true;
}
bool AAskirWorldPortal::Interact_Implementation(AActor* Interactor){return Travel(Interactor);}
FText AAskirWorldPortal::GetInteractionText_Implementation() const{return InteractionLabel;}
