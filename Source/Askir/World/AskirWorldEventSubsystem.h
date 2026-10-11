#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AskirWorldEventSubsystem.generated.h"
USTRUCT(BlueprintType)struct FAskirWorldEvent{GENERATED_BODY()UPROPERTY(BlueprintReadOnly)FName EventID;UPROPERTY(BlueprintReadOnly)FText Description;UPROPERTY(BlueprintReadOnly)int32 DurationMinutes=60;UPROPERTY(BlueprintReadOnly)float Severity=0;};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAskirWorldEventCreated,const FAskirWorldEvent&,Event);
UCLASS()class ASKIR_API UAskirWorldEventSubsystem:public UWorldSubsystem{GENERATED_BODY()public:UFUNCTION(BlueprintCallable)void CreateEvent(FName ID,FText D,int32 Duration,float Severity);UFUNCTION(BlueprintCallable)void AdvanceEvents(int32 Minutes);UFUNCTION(BlueprintPure)TArray<FAskirWorldEvent>GetActiveEvents()const{return ActiveEvents;}UPROPERTY(BlueprintAssignable)FAskirWorldEventCreated OnWorldEventCreated;private:UPROPERTY()TArray<FAskirWorldEvent>ActiveEvents;};
