#include "AskirCompanionComponent.h"

void UAskirCompanionComponent::Praise(float Amount) { Loyalty=FMath::Clamp(Loyalty+Amount,0.f,100.f); }
void UAskirCompanionComponent::TrainTogether(float Hours) { Bond=FMath::Clamp(Bond+FMath::Max(0.f,Hours)*5.f,0.f,100.f); }
void UAskirCompanionComponent::IncreaseStress(float Amount) { Stress=FMath::Clamp(Stress+Amount,0.f,100.f); }
void UAskirCompanionComponent::Comfort(float Amount) { Stress=FMath::Max(0.f,Stress-FMath::Max(0.f,Amount)); }
void UAskirCompanionComponent::LoseLoyalty(float Amount) { Loyalty=FMath::Max(0.f,Loyalty-FMath::Max(0.f,Amount)); }
