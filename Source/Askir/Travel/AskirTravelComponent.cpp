#include "AskirTravelComponent.h"

void UAskirTravelComponent::Walk(float Distance) { TravelDistance+=FMath::Max(0.f,Distance);TravelFatigue=FMath::Clamp(TravelFatigue+FMath::Max(0.f,Distance)*0.002f,0.f,100.f); }
void UAskirTravelComponent::ConsumeSupplies(float Amount) { Supplies=FMath::Max(0.f,Supplies-FMath::Max(0.f,Amount)); }
void UAskirTravelComponent::Rest(float Hours) { TravelFatigue=FMath::Max(0.f,TravelFatigue-FMath::Max(0.f,Hours)*12.f); }
void UAskirTravelComponent::Resupply(float Amount) { Supplies=FMath::Clamp(Supplies+FMath::Max(0.f,Amount),0.f,100.f); }
void UAskirTravelComponent::FastTravel(float Distance) { TravelDistance+=FMath::Max(0.f,Distance);Supplies=FMath::Max(0.f,Supplies-Distance*0.001f); }
