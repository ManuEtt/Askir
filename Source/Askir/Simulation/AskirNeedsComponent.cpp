#include "Simulation/AskirNeedsComponent.h"
namespace
{
    float SafeNeed(float Value){return FMath::IsFinite(Value)?FMath::Clamp(Value,0.f,100.f):0.f;}
    float SafeRate(float Value){return FMath::IsFinite(Value)?FMath::Max(0.f,Value):0.f;}
}
UAskirNeedsComponent::UAskirNeedsComponent(){PrimaryComponentTick.bCanEverTick=false;}
void UAskirNeedsComponent::AdvanceNeeds(float Minutes)
{
    if(!FMath::IsFinite(Minutes)||Minutes<=0)return;
    Needs.Hunger=FMath::Clamp(SafeNeed(Needs.Hunger)+SafeRate(HungerRate)*Minutes,0.f,100.f);
    Needs.Thirst=FMath::Clamp(SafeNeed(Needs.Thirst)+SafeRate(ThirstRate)*Minutes,0.f,100.f);
    Needs.Fatigue=FMath::Clamp(SafeNeed(Needs.Fatigue)+SafeRate(FatigueRate)*Minutes,0.f,100.f);
    Needs.Stress=FMath::Clamp(SafeNeed(Needs.Stress)+(Needs.Safety<.3f?.04f:.01f)*Minutes,0.f,100.f);
}
void UAskirNeedsComponent::SatisfyHunger(float Amount)
{if(FMath::IsFinite(Amount)&&Amount>0)Needs.Hunger=FMath::Clamp(SafeNeed(Needs.Hunger)-Amount,0.f,100.f);}
void UAskirNeedsComponent::SatisfyThirst(float Amount)
{if(FMath::IsFinite(Amount)&&Amount>0)Needs.Thirst=FMath::Clamp(SafeNeed(Needs.Thirst)-Amount,0.f,100.f);}
void UAskirNeedsComponent::Rest(float Minutes)
{
    if(!FMath::IsFinite(Minutes)||Minutes<=0)return;
    Needs.Fatigue=FMath::Clamp(SafeNeed(Needs.Fatigue)-Minutes*.12f,0.f,100.f);
    Needs.Stress=FMath::Clamp(SafeNeed(Needs.Stress)-Minutes*.03f,0.f,100.f);
}
float UAskirNeedsComponent::GetUrgency()const{return FMath::Max3(SafeNeed(Needs.Hunger),SafeNeed(Needs.Thirst),SafeNeed(Needs.Fatigue));}
