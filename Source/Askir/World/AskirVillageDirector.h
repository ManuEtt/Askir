#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/AskirWorldClockComponent.h"
#include "AskirVillageDirector.generated.h"
class UAskirDailyRoutineComponent;
UCLASS(Blueprintable)
class ASKIR_API AAskirVillageDirector:public AActor
{
 GENERATED_BODY()
public:
 AAskirVillageDirector();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UAskirWorldClockComponent> Clock;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Village") TArray<TObjectPtr<AActor>> Residents;
 UFUNCTION(BlueprintCallable) void RefreshResidents();
 UFUNCTION(BlueprintCallable) void AdvanceVillageHours(int32 Hours);
protected:
 virtual void BeginPlay() override;
 UFUNCTION() void HandleHour(int32 Day,int32 Hour);
};
