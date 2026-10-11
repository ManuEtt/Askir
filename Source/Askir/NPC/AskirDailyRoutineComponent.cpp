#include "NPC/AskirDailyRoutineComponent.h"
UAskirDailyRoutineComponent::UAskirDailyRoutineComponent(){PrimaryComponentTick.bCanEverTick=false;}
void UAskirDailyRoutineComponent::EvaluateAtHour(int32 Hour)
{
 const int32 Normalized=((Hour%24)+24)%24;
 const FAskirScheduleEntry* Best=nullptr;
 int32 BestAge=25;
 for(const FAskirScheduleEntry& Entry:Schedule)
 {
  const int32 Age=(Normalized-((Entry.StartHour%24+24)%24)+24)%24;
  if(Age<BestAge){BestAge=Age;Best=&Entry;}
 }
 if(Best && (CurrentActivity!=Best->Activity||CurrentDestinationTag!=Best->DestinationTag))
 {
  CurrentActivity=Best->Activity;CurrentDestinationTag=Best->DestinationTag;
  OnRoutineChanged.Broadcast(CurrentActivity,CurrentDestinationTag);
 }
}
void UAskirDailyRoutineComponent::UseVillagerSchedule()
{
 Schedule.Reset();
 auto Add=[this](int32 H,EAskirDailyActivity A,const TCHAR* T)
 {FAskirScheduleEntry E;E.StartHour=H;E.Activity=A;E.DestinationTag=FName(T);Schedule.Add(E);};
 Add(0,EAskirDailyActivity::Sleep,TEXT("Home"));
 Add(7,EAskirDailyActivity::Eat,TEXT("Inn"));
 Add(8,EAskirDailyActivity::Work,TEXT("Workplace"));
 Add(12,EAskirDailyActivity::Eat,TEXT("Inn"));
 Add(13,EAskirDailyActivity::Work,TEXT("Workplace"));
 Add(18,EAskirDailyActivity::Leisure,TEXT("Market"));
 Add(21,EAskirDailyActivity::Sleep,TEXT("Home"));
}
void UAskirDailyRoutineComponent::UseGuardSchedule()
{
 Schedule.Reset();
 for(int32 H : {0,6,14,22})
 {FAskirScheduleEntry E;E.StartHour=H;E.Activity=(H==6||H==14)?EAskirDailyActivity::Guard:EAskirDailyActivity::Sleep;
 E.DestinationTag=E.Activity==EAskirDailyActivity::Guard?FName(TEXT("GuardPost")):FName(TEXT("Barracks"));Schedule.Add(E);}
}
