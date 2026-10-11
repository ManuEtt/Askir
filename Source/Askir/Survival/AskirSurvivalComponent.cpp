#include "AskirSurvivalComponent.h"

void UAskirSurvivalComponent::AddWarmth(float Value) { Warmth=FMath::Clamp(Warmth+Value,0.f,100.f); }
void UAskirSurvivalComponent::AddWetness(float Value) { Wetness=FMath::Clamp(Wetness+Value,0.f,100.f); }
void UAskirSurvivalComponent::ApplyExposure(float Hours) { Exposure=FMath::Clamp(Exposure+FMath::Max(0.f,Hours)*(100.f-Warmth)/100.f,0.f,100.f); }
void UAskirSurvivalComponent::DryOff(float Amount) { Wetness=FMath::Max(0.f,Wetness-FMath::Max(0.f,Amount)); }
void UAskirSurvivalComponent::RestAtFire(float Hours) { Warmth=FMath::Clamp(Warmth+FMath::Max(0.f,Hours)*15.f,0.f,100.f);Wetness=FMath::Max(0.f,Wetness-Hours*20.f); }
