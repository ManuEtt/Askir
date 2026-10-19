#include "Travel/AskirWorldTravelSubsystem.h"
#include "Travel/AskirTravelRules.h"
#include "Game/AskirPlayerController.h"
#include "Characters/AskirCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

void UAskirWorldTravelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
 Super::Initialize(Collection);
 if(GEngine)TravelFailureHandle=GEngine->OnTravelFailure().AddUObject(this,&UAskirWorldTravelSubsystem::HandleTravelFailure);
}
void UAskirWorldTravelSubsystem::Deinitialize()
{
 if(GEngine&&TravelFailureHandle.IsValid())GEngine->OnTravelFailure().Remove(TravelFailureHandle);
 ClearPendingTravel();
 Super::Deinitialize();
}
bool UAskirWorldTravelSubsystem::HasPendingTravel() const
{
 return !PendingDestination.IsNone()||PendingSnapshot!=nullptr;
}
bool UAskirWorldTravelSubsystem::IsUsableWorldPackage(const FString& Package,FName ExpectedMap)
{
 if(ExpectedMap.IsNone()||!FPackageName::IsValidLongPackageName(Package)||!Package.StartsWith(TEXT("/Game/"))
  ||FName(*FPackageName::GetShortName(Package))!=ExpectedMap||!FPackageName::DoesPackageExist(Package))return false;
 const FString ObjectPath=Package+TEXT(".")+FPackageName::GetShortName(Package);
 const auto* World=LoadObject<UWorld>(nullptr,*ObjectPath,nullptr,LOAD_NoWarn|LOAD_Quiet);
 return World&&World->PersistentLevel;
}
void UAskirWorldTravelSubsystem::HandleTravelFailure(UWorld* World,ETravelFailure::Type FailureType,const FString& Error)
{
 if(!HasPendingTravel())return;
 if(World&&World->GetGameInstance()!=GetGameInstance())return;
 // The failure callback can still name the old world after Browse has torn it down.
 if(auto* ActiveWorld=GetGameInstance()?GetGameInstance()->GetWorld():nullptr)World=ActiveWorld;
 UE_LOG(LogTemp,Error,TEXT("Askir travel failed (%d): %s"),static_cast<int32>(FailureType),*Error);
 const FName CurrentMap=World?FName(*UGameplayStatics::GetCurrentLevelName(World,true)):NAME_None;
 const auto* Player=World?Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(World,0)):nullptr;
 const bool bPlayerReady=IsValid(Player)&&!Player->IsActorBeingDestroyed()&&IsValid(Player->GetController());
 if(AskirTravelRules::CanResumeLiveSource(bSourceStateAuthoritative,
  World&&RequestOriginWorld.Get()==World&&!RequestOriginMap.IsNone()&&CurrentMap==RequestOriginMap,
  !World||World->bIsTearingDown,bPlayerReady))
 {
  // OpenLevel failed before the source was torn down; live source remains authoritative.
  ClearPendingTravel();
  LastTravelError=NSLOCTEXT("Askir","TravelStartFailed","Die Reise konnte nicht gestartet werden. Bitte versuche es erneut.");
  if(auto* PC=Cast<AAskirPlayerController>(UGameplayStatics::GetPlayerController(World,0)))PC->ShowNotification(LastTravelError);
 }
 else MarkArrivalFailed(NSLOCTEXT("Askir","TravelInterrupted","Die Reise wurde unterbrochen. Versuche es erneut oder kehre zurück."));
}
void UAskirWorldTravelSubsystem::QueueArrival(FName Destination,FName FromMap)
{
 // Compatibility request; never erase another request's strong snapshot or stale flags.
 if(HasPendingTravel()||Destination.IsNone())return;
 ClearPendingTravel();
 PendingDestination=Destination;
 PendingArrivalFrom=FromMap;
}
bool UAskirWorldTravelSubsystem::QueueSnapshot(FName Destination,FName FromMap,UAskirSaveGame* Snapshot,bool bRestorePosition)
{
 FString Reason;
 const bool bValidSnapshot=IsValid(Snapshot)&&Snapshot->ValidateSnapshot(Reason);
 const bool bMatchesSnapshot=bValidSnapshot&&(bRestorePosition?Snapshot->SavedMapName==Destination:
  (!FromMap.IsNone()&&Snapshot->SavedMapName==FromMap));
 if(!AskirTravelRules::CanQueue(HasPendingTravel(),!Destination.IsNone()&&Destination!=FromMap,bValidSnapshot&&bMatchesSnapshot))return false;
 ClearPendingTravel();
 PendingDestination=Destination;
 PendingArrivalFrom=FromMap;
 PendingSnapshot=Snapshot;
 bPendingRestorePosition=bRestorePosition;
 bRestoreTravelSnapshot=true;
 auto* ActiveWorld=GetGameInstance()?GetGameInstance()->GetWorld():nullptr;
 RequestOriginWorld=ActiveWorld;
 RequestOriginMap=ActiveWorld?FName(*UGameplayStatics::GetCurrentLevelName(ActiveWorld,true)):FromMap;
 bSourceStateAuthoritative=true;
 return true;
}
void UAskirWorldTravelSubsystem::MarkArrivalFailed(FText Reason)
{
 bArrivalFailed=true;
 LastTravelError=Reason;
 UE_LOG(LogTemp,Error,TEXT("Askir arrival blocked: %s"),*Reason.ToString());
}
void UAskirWorldTravelSubsystem::ResumeArrival()
{
 bArrivalFailed=false;
 LastTravelError=FText::GetEmpty();
}
bool UAskirWorldTravelSubsystem::RedirectToSavedWorld(FString& OutPackage)
{
 if(!PendingSnapshot||PendingSnapshot->SavedMapName.IsNone())return false;
 FString Package=PendingSnapshot->SavedMapPackageName;
 if(Package.IsEmpty())Package=FString::Printf(TEXT("/Game/Askir/WorldV42/%s"),*PendingSnapshot->SavedMapName.ToString());
 if(!IsUsableWorldPackage(Package,PendingSnapshot->SavedMapName))return false;
 PendingDestination=PendingSnapshot->SavedMapName;
 PendingArrivalFrom=NAME_None;
 PendingArrivalPortalID=NAME_None;
 PendingDeparturePortalID=NAME_None;
 bPendingRestorePosition=true;
 // The current failed-arrival world has not restored the player's timeline.
 // A failed rollback must therefore retain the snapshot too.
 bSourceStateAuthoritative=false;
 ResumeArrival();
 OutPackage=MoveTemp(Package);
 return true;
}
bool UAskirWorldTravelSubsystem::ConsumeArrival(FName CurrentMap,FName& OutFromMap)
{
 // A snapshot must be restored before acknowledgement; a compatibility arrival has none.
 if(PendingDestination.IsNone()||PendingDestination!=CurrentMap||PendingSnapshot)return false;
 OutFromMap=PendingArrivalFrom;
 ClearPendingTravel();
 return true;
}
void UAskirWorldTravelSubsystem::ClearPendingTravel()
{
 PendingDestination=NAME_None;
 PendingArrivalFrom=NAME_None;
 bRestoreTravelSnapshot=false;
 PendingSnapshot=nullptr;
 bPendingRestorePosition=false;
 PendingArrivalPortalID=NAME_None;
 PendingDeparturePortalID=NAME_None;
 bArrivalFailed=false;
 LastTravelError=FText::GetEmpty();
 RequestOriginMap=NAME_None;
 RequestOriginWorld.Reset();
 bSourceStateAuthoritative=false;
}
