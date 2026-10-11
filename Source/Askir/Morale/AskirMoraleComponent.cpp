#include "AskirMoraleComponent.h"

void UAskirMoraleComponent::Encourage(float Amount) { Morale=FMath::Clamp(Morale+Amount,0.f,100.f); }
void UAskirMoraleComponent::Intimidate(float Amount) { Fear=FMath::Clamp(Fear+Amount,0.f,100.f); }
void UAskirMoraleComponent::RecoverFear(float Amount) { Fear=FMath::Max(0.f,Fear-FMath::Max(0.f,Amount)); }
void UAskirMoraleComponent::TrainCourage(float Amount) { Courage=FMath::Clamp(Courage+Amount,0.f,100.f); }
void UAskirMoraleComponent::BreakMorale(float Amount) { Morale=FMath::Max(0.f,Morale-FMath::Max(0.f,Amount)); }
