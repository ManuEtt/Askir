#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/AskirTypes.h"
#include "AskirEntity.generated.h"
UCLASS()
class ASKIR_API AAskirEntity:public AActor
{
    GENERATED_BODY()
public:
    AAskirEntity();
    UPROPERTY(EditAnywhere,BlueprintReadOnly,SaveGame) FAskirEntityID EntityID;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) FText DisplayName;
};