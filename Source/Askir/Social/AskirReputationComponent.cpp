#include "Social/AskirReputationComponent.h"
#include "Core/AskirCheckedMath.h"
UAskirReputationComponent::UAskirReputationComponent(){PrimaryComponentTick.bCanEverTick=false;}
int32 UAskirReputationComponent::GetReputation(FName Faction) const
{for(const auto& S:Standings)if(S.Faction==Faction)return S.Reputation;return 0;}
void UAskirReputationComponent::SetReputation(FName Faction,int32 Value)
{
 if(Faction.IsNone())return;
 const int32 Safe=FMath::Clamp(Value,-100,100);
 for(auto& S:Standings)if(S.Faction==Faction)
 {if(S.Reputation!=Safe){S.Reputation=Safe;OnReputationChanged.Broadcast(Faction,Safe);}return;}
 FAskirFactionStanding S;S.Faction=Faction;S.Reputation=Safe;Standings.Add(S);
 OnReputationChanged.Broadcast(Faction,Safe);
}
int32 UAskirReputationComponent::ChangeReputation(FName Faction,int32 Delta)
{
 const int32 NewValue=AskirCheckedMath::ClampAdd(GetReputation(Faction),Delta,-100,100);
 SetReputation(Faction,NewValue);return NewValue;
}
bool UAskirReputationComponent::IsHostile(FName Faction) const
{return GetReputation(Faction)<HostileBelow;}
bool UAskirReputationComponent::IsFriendly(FName Faction) const
{return GetReputation(Faction)>=FriendlyFrom;}
float UAskirReputationComponent::GetMerchantPriceMultiplier(FName Faction) const
{return FMath::Clamp(1.f-GetReputation(Faction)*0.0025f,0.75f,1.25f);}
