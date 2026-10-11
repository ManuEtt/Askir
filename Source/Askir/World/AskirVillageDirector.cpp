#include "World/AskirVillageDirector.h"
#include "NPC/AskirDailyRoutineComponent.h"
AAskirVillageDirector::AAskirVillageDirector()
{
 PrimaryActorTick.bCanEverTick=false;
 Clock=CreateDefaultSubobject<UAskirWorldClockComponent>(TEXT("WorldClock"));
}
void AAskirVillageDirector::BeginPlay()
{
 Super::BeginPlay();
 Clock->OnHourChanged.AddDynamic(this,&AAskirVillageDirector::HandleHour);
 RefreshResidents();
}
void AAskirVillageDirector::HandleHour(int32 Day,int32 Hour)
{
 for(AActor* Resident:Residents)
  if(IsValid(Resident))
   if(auto* Routine=Resident->FindComponentByClass<UAskirDailyRoutineComponent>())
    Routine->EvaluateAtHour(Hour);
}
void AAskirVillageDirector::RefreshResidents(){HandleHour(Clock->CurrentDay,Clock->CurrentHour);}
void AAskirVillageDirector::AdvanceVillageHours(int32 Hours){Clock->AdvanceHours(Hours);}
