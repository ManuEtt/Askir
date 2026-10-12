#include "Travel/AskirWorldTravelSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

void UAskirWorldTravelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
 Super::Initialize(Collection);
 if(GEngine)TravelFailureHandle=GEngine->OnTravelFailure().AddUObject(this,&UAskirWorldTravelSubsystem::HandleTravelFailure);
}
void UAskirWorldTravelSubsystem::Deinitialize()
{
 if(GEngine && TravelFailureHandle.IsValid())GEngine->OnTravelFailure().Remove(TravelFailureHandle);
 ClearPendingTravel();
 Super::Deinitialize();
}
void UAskirWorldTravelSubsystem::HandleTravelFailure(UWorld* World,ETravelFailure::Type FailureType,const FString& Error)
{
 if(World && World->GetGameInstance()!=GetGameInstance())return;
 UE_LOG(LogTemp,Error,TEXT("Askir travel failed (%d): %s"),static_cast<int32>(FailureType),*Error);
 ClearPendingTravel();
}
void UAskirWorldTravelSubsystem::QueueArrival(FName Destination, FName FromMap)
{
 PendingDestination=Destination;
 PendingArrivalFrom=FromMap;
 PendingArrivalPortalID=NAME_None;
 PendingSnapshot=nullptr;
 bPendingRestorePosition=false;
}
void UAskirWorldTravelSubsystem::QueueSnapshot(FName Destination,FName FromMap,UAskirSaveGame* Snapshot,bool bRestorePosition)
{
 QueueArrival(Destination,FromMap);
 PendingSnapshot=Snapshot;
 bPendingRestorePosition=bRestorePosition;
 bRestoreTravelSnapshot=false;
}
bool UAskirWorldTravelSubsystem::ConsumeArrival(FName CurrentMap, FName& OutFromMap)
{
 if(PendingDestination.IsNone() || PendingDestination!=CurrentMap)return false;
 OutFromMap=PendingArrivalFrom;
 ClearPendingTravel();
 return true;
}

void UAskirWorldTravelSubsystem::ClearPendingTravel()
{
 PendingDestination=NAME_None; PendingArrivalFrom=NAME_None; bRestoreTravelSnapshot=false;
 PendingSnapshot=nullptr; bPendingRestorePosition=false;
 PendingArrivalPortalID=NAME_None;
}
