#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Save/AskirSaveGame.h"
#include "Engine/EngineBaseTypes.h"
#include "AskirWorldTravelSubsystem.generated.h"

/** Transient travel request survives OpenLevel within the same game instance. */
UCLASS()
class ASKIR_API UAskirWorldTravelSubsystem : public UGameInstanceSubsystem
{
 GENERATED_BODY()
public:
 virtual void Initialize(FSubsystemCollectionBase& Collection) override;
 virtual void Deinitialize() override;
 UPROPERTY(BlueprintReadOnly, Category="Askir|World") bool bRestoreTravelSnapshot = false;
 UPROPERTY(BlueprintReadOnly, Category="Askir|World") FName PendingDestination = NAME_None;
 UPROPERTY(BlueprintReadOnly, Category="Askir|World") FName PendingArrivalFrom = NAME_None;
 /** The active timeline ledger belongs to the session, never another save slot. */
 UPROPERTY() TMap<FName,FAskirLootState> LootByMap;
 UPROPERTY() TMap<FName,FAskirMapMerchantState> MerchantsByMap;
 UPROPERTY() TObjectPtr<UAskirSaveGame> PendingSnapshot;
 bool bPendingRestorePosition=false;
 FName PendingArrivalPortalID=NAME_None;
 FName PendingDeparturePortalID=NAME_None;
 double ArrivalGraceUntilSeconds=-1.0;
 UPROPERTY(BlueprintReadOnly,Category="Askir|World") bool bArrivalFailed=false;
 UPROPERTY(BlueprintReadOnly,Category="Askir|World") FText LastTravelError;
 UFUNCTION(BlueprintPure,Category="Askir|World") bool HasPendingTravel() const;
 UFUNCTION(BlueprintPure,Category="Askir|World") bool IsGameplayBlocked() const { return HasPendingTravel(); }
 /** Reject arbitrary assets: travel destinations must resolve to a real UWorld. */
 static bool IsUsableWorldPackage(const FString& Package,FName ExpectedMap);
 bool QueueSnapshot(FName Destination,FName FromMap,UAskirSaveGame* Snapshot,bool bRestorePosition);
 void MarkArrivalFailed(FText Reason);
 void ResumeArrival();
 /** Preserve the authoritative handoff while returning to its captured world. */
 bool RedirectToSavedWorld(FString& OutPackage);
 UFUNCTION(BlueprintCallable, Category="Askir|World") void QueueArrival(FName Destination, FName FromMap);
 UFUNCTION(BlueprintCallable, Category="Askir|World") void ClearPendingTravel();
 UFUNCTION(BlueprintCallable, Category="Askir|World") bool ConsumeArrival(FName CurrentMap, FName& OutFromMap);
private:
 FName RequestOriginMap=NAME_None;
 TWeakObjectPtr<UWorld> RequestOriginWorld;
 bool bSourceStateAuthoritative=false;
 FDelegateHandle TravelFailureHandle;
 void HandleTravelFailure(UWorld* World,ETravelFailure::Type FailureType,const FString& Error);
};
