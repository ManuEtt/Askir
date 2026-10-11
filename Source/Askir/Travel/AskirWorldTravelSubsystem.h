#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AskirWorldTravelSubsystem.generated.h"

/** Transient travel request survives OpenLevel within the same game instance. */
UCLASS()
class ASKIR_API UAskirWorldTravelSubsystem : public UGameInstanceSubsystem
{
 GENERATED_BODY()
public:
 UPROPERTY(BlueprintReadOnly, Category="Askir|World") bool bRestoreTravelSnapshot = false;
 UPROPERTY(BlueprintReadOnly, Category="Askir|World") FName PendingDestination = NAME_None;
 UPROPERTY(BlueprintReadOnly, Category="Askir|World") FName PendingArrivalFrom = NAME_None;
 UFUNCTION(BlueprintCallable, Category="Askir|World") void QueueArrival(FName Destination, FName FromMap);
 UFUNCTION(BlueprintCallable, Category="Askir|World") void ClearPendingTravel();
 UFUNCTION(BlueprintCallable, Category="Askir|World") bool ConsumeArrival(FName CurrentMap, FName& OutFromMap);
};
