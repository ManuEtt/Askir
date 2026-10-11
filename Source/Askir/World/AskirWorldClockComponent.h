#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirWorldClockComponent.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAskirWorldHourChanged,int32,Day,int32,Hour);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAskirWorldEvent,FName,EventID);
USTRUCT(BlueprintType)
struct FAskirTimedWorldEvent
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName EventID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Hour=12;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bRepeatDaily=true;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) int32 LastTriggeredDay=-1;
};
UCLASS(ClassGroup=(Askir),meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirWorldClockComponent:public UActorComponent
{
 GENERATED_BODY()
public:
 UAskirWorldClockComponent();
 virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") float RealSecondsPerGameHour=90.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") bool bPaused=false;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") int32 CurrentDay=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") int32 CurrentHour=8;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") TArray<FAskirTimedWorldEvent> TimedEvents;
 UPROPERTY(BlueprintAssignable,Category="Askir|World") FAskirWorldHourChanged OnHourChanged;
 UPROPERTY(BlueprintAssignable,Category="Askir|World") FAskirWorldEvent OnWorldEvent;
 UFUNCTION(BlueprintCallable,Category="Askir|World") void AdvanceHours(int32 Hours);
 UFUNCTION(BlueprintCallable,Category="Askir|World") void SetClock(int32 Day,int32 Hour);
 UFUNCTION(BlueprintPure,Category="Askir|World") float GetDaylightFactor() const;
private:
 float AccumulatedSeconds=0.f;
 void CheckEvents();
};
