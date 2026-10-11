#include "Combat/AskirGuardComponent.h"
#include "Engine/World.h"
UAskirGuardComponent::UAskirGuardComponent(){PrimaryComponentTick.bCanEverTick=true;}
void UAskirGuardComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
 Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
 if(!bGuarding && DeltaTime>0.f) Stamina=FMath::Clamp(Stamina+FMath::Max(0.f,RegenPerSecond)*DeltaTime,0.f,FMath::Max(1.f,MaxStamina));
}
bool UAskirGuardComponent::StartGuard()
{
 const UWorld* World=GetWorld();
 if(!World||World->GetTimeSeconds()<GuardLockUntil||Stamina<=0.f)return false;
 bGuarding=true;return true;
}
void UAskirGuardComponent::StopGuard(){bGuarding=false;}
float UAskirGuardComponent::AbsorbIncomingDamage(float IncomingDamage)
{
 const float Damage=FMath::Max(0.f,IncomingDamage);
 if(!bGuarding||Damage<=0.f)return Damage;
 const float Cost=Damage*FMath::Max(0.f,StaminaCostPerDamage);
 const float Fraction=Cost>0.f?FMath::Clamp(Stamina/Cost,0.f,1.f):1.f;
 Stamina=FMath::Max(0.f,Stamina-Cost);
 const float Reduced=Damage*(1.f-FMath::Clamp(DamageReduction,0.f,1.f)*Fraction);
 if(Stamina<=0.f)
 {
  bGuarding=false;
  if(const UWorld* World=GetWorld())GuardLockUntil=World->GetTimeSeconds()+FMath::Max(0.f,GuardBreakCooldown);
  OnGuardBroken.Broadcast();
 }
 return Reduced;
}
void UAskirGuardComponent::RestoreStamina(){Stamina=FMath::Max(1.f,MaxStamina);bGuarding=false;GuardLockUntil=0.f;}
float UAskirGuardComponent::GetStaminaPercent()const{return FMath::Clamp(Stamina/FMath::Max(1.f,MaxStamina),0.f,1.f);}
