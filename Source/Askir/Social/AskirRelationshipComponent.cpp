#include "Social/AskirRelationshipComponent.h"
#include "Core/AskirEntity.h"
UAskirRelationshipComponent::UAskirRelationshipComponent(){PrimaryComponentTick.bCanEverTick=false;}
void UAskirRelationshipComponent::ModifyRelationship(AActor*O,float A,float T,float F,float R,float S){if(!FMath::IsFinite(A)||!FMath::IsFinite(T)||!FMath::IsFinite(F)||!FMath::IsFinite(R)||!FMath::IsFinite(S))return;if(AAskirEntity*E=Cast<AAskirEntity>(O)){auto&V=Relationships.FindOrAdd(E->EntityID);V.Affinity=FMath::Clamp(V.Affinity+A,-100.f,100.f);V.Trust=FMath::Clamp(V.Trust+T,-100.f,100.f);V.Fear=FMath::Clamp(V.Fear+F,0.f,100.f);V.Respect=FMath::Clamp(V.Respect+R,-100.f,100.f);V.Suspicion=FMath::Clamp(V.Suspicion+S,0.f,100.f);}}
FAskirRelationship UAskirRelationshipComponent::GetRelationship(AActor*O)const{if(const AAskirEntity*E=Cast<AAskirEntity>(O))if(const auto*V=Relationships.Find(E->EntityID))return *V;return FAskirRelationship();}
