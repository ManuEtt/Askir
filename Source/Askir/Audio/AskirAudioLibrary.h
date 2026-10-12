#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AskirAudioLibrary.generated.h"

/** Optional feedback assets generated/imported by the V77 pipeline. Missing cues are silent. */
UCLASS()
class ASKIR_API UAskirAudioLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Askir|Audio", meta=(WorldContext="WorldContext"))
    static void PlayCue(UObject* WorldContext,FName Cue);
    UFUNCTION(BlueprintCallable, Category="Askir|Audio")
    static void PlayWorldCue(AActor* Source,FName Cue);
};
