#include "AskirStealthComponent.h"

void UAskirStealthComponent::MakeNoise(float Amount) { Noise=FMath::Clamp(Noise+Amount,0.f,100.f); }
void UAskirStealthComponent::Crouch() { Visibility=FMath::Max(0.f,Visibility-25.f); }
void UAskirStealthComponent::StandUp() { Visibility=FMath::Min(100.f,Visibility+25.f); }
void UAskirStealthComponent::RaiseSuspicion(float Amount) { Suspicion=FMath::Clamp(Suspicion+Amount,0.f,100.f); }
void UAskirStealthComponent::CalmDown(float Amount) { Suspicion=FMath::Max(0.f,Suspicion-FMath::Max(0.f,Amount)); }
