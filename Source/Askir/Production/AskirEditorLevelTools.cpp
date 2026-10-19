#include "Production/AskirEditorLevelTools.h"
#include "Characters/AskirCharacter.h"
#include "Components/CapsuleComponent.h"
#include "CollisionShape.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/WorldSettings.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"

#if WITH_EDITOR
#include "Builders/CubeBuilder.h"
#include "Components/BrushComponent.h"
#include "Engine/Model.h"
#include "Editor.h"
#endif

ANavMeshBoundsVolume* UAskirEditorLevelTools::CreateNavigationBounds(UObject* WorldContextObject, FVector Center, FVector Extent)
{
#if WITH_EDITOR
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (!World || World->WorldType != EWorldType::Editor || Extent.ContainsNaN()
        || Extent.X <= 0.f || Extent.Y <= 0.f || Extent.Z <= 0.f || Center.ContainsNaN()) return nullptr;
    FActorSpawnParameters Params;
    Params.ObjectFlags |= RF_Transactional;
    ANavMeshBoundsVolume* Volume = World->SpawnActor<ANavMeshBoundsVolume>(Center, FRotator::ZeroRotator, Params);
    if (!Volume) return nullptr;
    Volume->Brush = NewObject<UModel>(Volume, NAME_None, RF_Transactional);
    Volume->Brush->Initialize(nullptr, true);
    UCubeBuilder* Builder = NewObject<UCubeBuilder>(Volume, NAME_None, RF_Transactional);
    Volume->BrushBuilder = Builder;
    Builder->X = Extent.X * 2.f;
    Builder->Y = Extent.Y * 2.f;
    Builder->Z = Extent.Z * 2.f;
    if (!Builder->Build(World, Volume))
    {
        Volume->Destroy();
        return nullptr;
    }
    Volume->SetActorLocation(Center);
    Volume->GetBrushComponent()->Brush = Volume->Brush;
    Volume->PostEditChange();
    Volume->MarkPackageDirty();
    if (UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
        Navigation->OnNavigationBoundsUpdated(Volume);
    return Volume;
#else
    return nullptr;
#endif
}

bool UAskirEditorLevelTools::BuildEditorNavigation(UObject* WorldContextObject)
{
#if WITH_EDITOR
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (!World || World->WorldType != EWorldType::Editor) return false;
    UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!Navigation)
    {
        FNavigationSystem::AddNavigationSystemToWorld(*World, FNavigationSystemRunMode::EditorMode);
        Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    }
    if (!Navigation) return false;
    Navigation->Build();
    return Navigation->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) != nullptr;
#else
    return false;
#endif
}

bool UAskirEditorLevelTools::HasWalkableGroundBelow(AActor* SpawnPoint)
{
    if (!IsValid(SpawnPoint) || !SpawnPoint->GetWorld()) return false;
    const FVector Start = SpawnPoint->GetActorLocation();
    if (!FMath::IsFinite(Start.X) || !FMath::IsFinite(Start.Y) || !FMath::IsFinite(Start.Z)) return false;
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(AskirSpawnGroundValidation), false, SpawnPoint);
    return SpawnPoint->GetWorld()->LineTraceSingleByChannel(Hit, Start, Start - FVector(0.f, 0.f, 500.f), ECC_Visibility, Params)
        && Hit.bBlockingHit && Hit.ImpactNormal.Z >= 0.7f;
}

FVector UAskirEditorLevelTools::GetPlayerCapsuleExtent()
{
    const AAskirCharacter* Character = GetDefault<AAskirCharacter>();
    const UCapsuleComponent* Capsule = Character ? Character->GetCapsuleComponent() : nullptr;
    if (!Capsule) return FVector::ZeroVector;
    return FVector(Capsule->GetUnscaledCapsuleRadius(), Capsule->GetUnscaledCapsuleRadius(), Capsule->GetUnscaledCapsuleHalfHeight());
}

bool UAskirEditorLevelTools::IsPlayerArrivalClear(UObject* WorldContextObject, FVector Location)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    const FVector Extent = GetPlayerCapsuleExtent();
    if (!World || Location.ContainsNaN() || !FMath::IsFinite(Location.X) || !FMath::IsFinite(Location.Y)
        || !FMath::IsFinite(Location.Z) || !FMath::IsFinite(Extent.X) || !FMath::IsFinite(Extent.Z)
        || Extent.X <= 0.f || Extent.Z < Extent.X || Extent.Z > 500.f) return false;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(AskirArrivalValidation), false);
    if (World->OverlapBlockingTestByChannel(Location, FQuat::Identity, ECC_Pawn,
        FCollisionShape::MakeCapsule(Extent.X, Extent.Z), Params)) return false;
    FHitResult Hit;
    return World->LineTraceSingleByChannel(Hit, Location, Location - FVector(0.f, 0.f, Extent.Z + 20.f), ECC_Visibility, Params)
        && Hit.bBlockingHit && Hit.ImpactNormal.Z >= 0.7f
        && Hit.Distance >= Extent.Z - 2.f && Hit.Distance <= Extent.Z + 20.f;
}

bool UAskirEditorLevelTools::ConfigureEditorGameMode(UObject* WorldContextObject, TSubclassOf<AGameModeBase> GameModeClass, bool bApply)
{
#if WITH_EDITOR
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (!World || World->WorldType != EWorldType::Editor || !GameModeClass) return false;
    AWorldSettings* Settings = World->GetWorldSettings();
    if (!Settings) return false;
    if (bApply)
    {
        Settings->DefaultGameMode = GameModeClass;
        Settings->MarkPackageDirty();
    }
    return Settings->DefaultGameMode == GameModeClass;
#else
    return false;
#endif
}
