#include "Travel/AskirWorldTravelSubsystem.h"
void UAskirWorldTravelSubsystem::QueueArrival(FName Destination, FName FromMap)
{
 PendingDestination=Destination;
 PendingArrivalFrom=FromMap;
}
bool UAskirWorldTravelSubsystem::ConsumeArrival(FName CurrentMap, FName& OutFromMap)
{
 if(PendingDestination.IsNone() || PendingDestination!=CurrentMap)return false;
 OutFromMap=PendingArrivalFrom;
 PendingDestination=NAME_None;
 PendingArrivalFrom=NAME_None;
 return true;
}

void UAskirWorldTravelSubsystem::ClearPendingTravel()
{
 PendingDestination=NAME_None; PendingArrivalFrom=NAME_None; bRestoreTravelSnapshot=false;
}
