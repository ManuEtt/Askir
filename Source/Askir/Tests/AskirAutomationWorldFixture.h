#pragma once
#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/Engine.h"
#include "Engine/URL.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

/** A initialized runtime world: actors must use SpawnActor, never NewObject. */
struct FAskirAutomationTestWorld
{
    UWorld* World=nullptr;
    FAskirAutomationTestWorld()
    {
        if(!GEngine)return;
        const UWorld::InitializationValues Init=UWorld::InitializationValues()
            .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(true)
            .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
            .SetTransactional(false);
        // CreateWorld performs initialization; initializing the same world again is invalid.
        World=UWorld::CreateWorld(EWorldType::Game,false,NAME_None,nullptr,true,ERHIFeatureLevel::Num,&Init);
        if(!World)return;
        GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
        World->InitializeActorsForPlay(FURL());
        World->BeginPlay();
    }
    ~FAskirAutomationTestWorld()
    {
        if(World){GEngine->DestroyWorldContext(World);World->DestroyWorld(false);}
    }
    template<class T> T* Spawn(const FVector& Position=FVector::ZeroVector)
    {
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        return World?World->SpawnActor<T>(Position,FRotator::ZeroRotator,Params):nullptr;
    }
    template<class T> T* AddComponent(AActor* Owner)
    {
        T* Component=NewObject<T>(Owner);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }
};
#endif
