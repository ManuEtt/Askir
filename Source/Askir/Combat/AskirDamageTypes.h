#pragma once
#include "CoreMinimal.h"
#include "AskirDamageTypes.generated.h"
UENUM(BlueprintType)
enum class EAskirDamageType:uint8{Physical,Slash,Pierce,Blunt,Fire,Frost,Lightning,Poison,Magic,TrueDamage};
UENUM(BlueprintType)
enum class EAskirHitZone:uint8{Unknown,Head,Torso,LeftArm,RightArm,LeftLeg,RightLeg};
USTRUCT(BlueprintType)
struct FAskirDamageInfo
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite) float Damage=0;
    UPROPERTY(BlueprintReadWrite) EAskirDamageType DamageType=EAskirDamageType::Physical;
    UPROPERTY(BlueprintReadWrite) EAskirHitZone HitZone=EAskirHitZone::Torso;
    UPROPERTY(BlueprintReadWrite) TObjectPtr<AActor> DamageCauser=nullptr;
    UPROPERTY(BlueprintReadWrite) FVector HitLocation=FVector::ZeroVector;
    /** Direction of incoming travel (attacker toward victim); zero means unknown. */
    UPROPERTY(BlueprintReadWrite) FVector HitDirection=FVector::ZeroVector;
};
