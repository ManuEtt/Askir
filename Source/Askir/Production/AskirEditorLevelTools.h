#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AskirEditorLevelTools.generated.h"

class ANavMeshBoundsVolume;
class AActor;
class AGameModeBase;

/** Editor generation helpers; runtime builds retain safe stubs and never depend on UnrealEd. */
UCLASS()
class ASKIR_API UAskirEditorLevelTools : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    /** Builds real brush geometry. Scaling an empty spawned volume does not create navigation bounds. */
    UFUNCTION(BlueprintCallable, Category="Askir|Editor", meta=(WorldContext="WorldContextObject"))
    static ANavMeshBoundsVolume* CreateNavigationBounds(UObject* WorldContextObject, FVector Center, FVector Extent);

    /** Synchronously asks the editor navigation system to rebuild. False is a generation failure. */
    UFUNCTION(BlueprintCallable, Category="Askir|Editor", meta=(WorldContext="WorldContextObject"))
    static bool BuildEditorNavigation(UObject* WorldContextObject);

    /** Use native WorldSettings access because UWorld.GetWorldSettings is not a stable Python API. */
    UFUNCTION(BlueprintCallable, Category="Askir|Editor", meta=(WorldContext="WorldContextObject"))
    static bool ConfigureEditorGameMode(UObject* WorldContextObject, TSubclassOf<AGameModeBase> GameModeClass, bool bApply = true);

    /** Real physics trace at a spawn point, used after save/reload instead of checking only a mesh name. */
    UFUNCTION(BlueprintPure, Category="Askir|Editor")
    static bool HasWalkableGroundBelow(AActor* SpawnPoint);
};
