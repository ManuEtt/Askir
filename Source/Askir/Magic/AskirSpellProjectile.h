#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magic/AskirMagicRules.h"
#include "AskirSpellProjectile.generated.h"
class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class ASKIR_API AAskirSpellProjectile : public AActor
{
    GENERATED_BODY()
public:
    AAskirSpellProjectile();
    virtual void Tick(float Delta) override;
    void InitializeSpell(APawn* Caster,AskirMagicRules::Element Element,float Damage,const FVector& Direction);
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Visual;
    UPROPERTY() TObjectPtr<UPointLightComponent> Glow;
    FVector Velocity=FVector::ZeroVector;
    AskirMagicRules::Element School=AskirMagicRules::Element::Fire;
    float Power=0.f;
    bool bResolved=false;
};
