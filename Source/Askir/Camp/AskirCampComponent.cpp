#include "AskirCampComponent.h"

void UAskirCampComponent::GatherFuel(float Amount) { Fuel=FMath::Max(0.f,Fuel+FMath::Max(0.f,Amount)); }
void UAskirCampComponent::StockFood(float Amount) { Food=FMath::Max(0.f,Food+FMath::Max(0.f,Amount)); }
void UAskirCampComponent::BuildShelter(float Amount) { Shelter=FMath::Clamp(Shelter+FMath::Max(0.f,Amount),0.f,100.f); }
void UAskirCampComponent::BurnFuel(float Hours) { Fuel=FMath::Max(0.f,Fuel-FMath::Max(0.f,Hours)); }
void UAskirCampComponent::EatRations(float Amount) { Food=FMath::Max(0.f,Food-FMath::Max(0.f,Amount)); }
