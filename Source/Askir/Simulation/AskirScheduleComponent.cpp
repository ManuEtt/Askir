#include "Simulation/AskirScheduleComponent.h"
UAskirScheduleComponent::UAskirScheduleComponent(){PrimaryComponentTick.bCanEverTick=false;}
EAskirRoutineActivity UAskirScheduleComponent::GetActivityAtMinute(int32 M)const{M=((M%1440)+1440)%1440;for(const auto&E:Schedule)if((E.StartMinute<=E.EndMinute&&M>=E.StartMinute&&M<E.EndMinute)||(E.StartMinute>E.EndMinute&&(M>=E.StartMinute||M<E.EndMinute)))return E.Activity;return EAskirRoutineActivity::Idle;}
FName UAskirScheduleComponent::GetLocationTagAtMinute(int32 M)const{M=((M%1440)+1440)%1440;for(const auto&E:Schedule)if((E.StartMinute<=E.EndMinute&&M>=E.StartMinute&&M<E.EndMinute)||(E.StartMinute>E.EndMinute&&(M>=E.StartMinute||M<E.EndMinute)))return E.LocationTag;return NAME_None;}
