#include "World/AskirWorldClockComponent.h"
UAskirWorldClockComponent::UAskirWorldClockComponent(){PrimaryComponentTick.bCanEverTick=true;}
void UAskirWorldClockComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* TickFunction)
{
 Super::TickComponent(DeltaTime,TickType,TickFunction);
 if(bPaused||!FMath::IsFinite(DeltaTime))return;
 AccumulatedSeconds+=FMath::Max(0.f,DeltaTime);
 const float Duration=FMath::Max(1.f,RealSecondsPerGameHour);
 const int32 Hours=FMath::Clamp(FMath::FloorToInt(AccumulatedSeconds/Duration),0,10000);
 if(Hours>0){AccumulatedSeconds-=Hours*Duration;AdvanceHours(Hours);}
}
void UAskirWorldClockComponent::AdvanceHours(int32 Hours)
{
 if(Hours<=0)return;
 Hours=FMath::Min(Hours,10000);
 for(int32 I=0;I<Hours;++I)
 {
  ++CurrentHour;
  if(CurrentHour>=24){CurrentHour=0;++CurrentDay;}
  OnHourChanged.Broadcast(CurrentDay,CurrentHour);
  CheckEvents();
 }
}
void UAskirWorldClockComponent::SetClock(int32 Day,int32 Hour)
{
 CurrentDay=FMath::Max(1,Day);
 CurrentHour=FMath::Clamp(Hour,0,23);
 AccumulatedSeconds=0.f;
 OnHourChanged.Broadcast(CurrentDay,CurrentHour);
 CheckEvents();
}
void UAskirWorldClockComponent::CheckEvents()
{
 for(auto& E:TimedEvents)
 {
  if(E.EventID.IsNone()||E.Hour!=CurrentHour||E.LastTriggeredDay==CurrentDay)continue;
  if(!E.bRepeatDaily&&E.LastTriggeredDay>=0)continue;
  E.LastTriggeredDay=CurrentDay;
  OnWorldEvent.Broadcast(E.EventID);
 }
}
float UAskirWorldClockComponent::GetDaylightFactor() const
{
 const float T=FMath::Abs(float(CurrentHour)-12.f)/12.f;
 return FMath::Clamp(1.f-T,0.f,1.f);
}
