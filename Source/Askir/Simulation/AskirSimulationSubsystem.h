#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/AskirTypes.h"
#include "AskirSimulationSubsystem.generated.h"
USTRUCT(BlueprintType)
struct FAskirWorldState
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) FAskirWorldTime Time;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) int64 SimulationTick=0;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) TMap<FName,float> GlobalValues;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) TMap<FName,int32> GlobalFlags;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAskirSimulationAdvanced);
UCLASS()
class ASKIR_API UAskirSimulationSubsystem:public UWorldSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void AdvanceSimulation(int32 Minutes);
    UFUNCTION(BlueprintPure) FAskirWorldState GetWorldState()const{return State;}
    UFUNCTION(BlueprintCallable) void RestoreWorldState(const FAskirWorldState& SavedState);
    UPROPERTY(BlueprintAssignable) FAskirSimulationAdvanced OnSimulationAdvanced;
private:
    FAskirWorldState State;
};