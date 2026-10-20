#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/EngineBaseTypes.h"
#include "AskirFrontendSubsystem.generated.h"
class AAskirGameMode;
class UAskirSaveGame;

/** Session navigation; disk slots and authoritative travel handoffs are never discarded. */
UCLASS()
class ASKIR_API UAskirFrontendSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    bool HasActiveSession() const { return bHasActiveSession; }
    bool IsFreshStartPending() const { return bFreshStartPending; }
    void MarkSessionActive() { bHasActiveSession=true; }
    bool CanContinue() const;
    bool ContinueGame(AAskirGameMode* Mode);
    bool CanStartNewGame(AAskirGameMode* Mode) const;
    /** First call needs an explicit discard confirmation if a session is active. */
    bool StartNewGame(AAskirGameMode* Mode,bool bDiscardConfirmed);
    /** Called only after the target world reports its actual runtime readiness. */
    bool CommitFreshStart(AAskirGameMode* Mode);
    FText GetLastError() const { return LastError; }
    static FString GetStartWorldPackage();
private:
    void HandleTravelFailure(UWorld* World,ETravelFailure::Type FailureType,const FString& Error);
    bool HasPendingTravel() const;
    bool TickFreshStartTimeout(float DeltaSeconds);
    void RecoverFreshStartFailure();
    bool bHasActiveSession=false;
    bool bFreshStartPending=false;
    UPROPERTY(Transient) TObjectPtr<UAskirSaveGame> FreshStartBackup;
    TWeakObjectPtr<UWorld> SourceWorld;
    FString FreshStartPackage;
    FText LastError;
    FDelegateHandle TravelFailureHandle;
    FDelegateHandle FreshStartTicker;
    double FreshStartDeadline=0.;
};
