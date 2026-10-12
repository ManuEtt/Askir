#include "Magic/AskirSpellProjectile.h"
#include "Magic/AskirMagicComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

AAskirSpellProjectile::AAskirSpellProjectile()
{
    PrimaryActorTick.bCanEverTick=true;
    Visual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpellVisual"));SetRootComponent(Visual);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if(Sphere.Succeeded())Visual->SetStaticMesh(Sphere.Object);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);Visual->SetRelativeScale3D(FVector(.15f));
    Glow=CreateDefaultSubobject<UPointLightComponent>(TEXT("SpellGlow"));Glow->SetupAttachment(Visual);Glow->SetIntensity(2500.f);Glow->SetAttenuationRadius(180.f);
    InitialLifeSpan=3.f;
}
void AAskirSpellProjectile::InitializeSpell(APawn* Caster,AskirMagicRules::Element Element,float Damage,const FVector& Direction)
{
    SetOwner(Caster);SetInstigator(Caster);School=Element;Power=Damage;Velocity=Direction.GetSafeNormal()*1400.f;
    const FLinearColor Colors[]={FLinearColor(1,.16f,.02f),FLinearColor(.1f,.6f,1),FLinearColor(.7f,.5f,1),FLinearColor(.5f,.3f,.1f),FLinearColor(1,.9f,.5f),FLinearColor(.5f,.05f,.7f)};
    Glow->SetLightColor(Colors[static_cast<int32>(School)]);
}
void AAskirSpellProjectile::Tick(float Delta)
{
    Super::Tick(Delta);
    if(bResolved||!GetWorld()||!IsValid(GetInstigator())){Destroy();return;}
    if(!FMath::IsFinite(Delta)||Delta<=0)return;
    const FVector Start=GetActorLocation(),End=Start+Velocity*Delta;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(AskirSpellProjectile),false,this);Query.AddIgnoredActor(GetInstigator());
    FHitResult Hit;
    if(GetWorld()->SweepSingleByChannel(Hit,Start,End,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(12.f),Query))
    {
        bResolved=true;
        if(AActor* Target=Hit.GetActor())UAskirMagicComponent::ApplySpellHit(GetInstigator(),Target,School,Power,Hit.ImpactPoint);
        Destroy();return;
    }
    SetActorLocation(End);
}
