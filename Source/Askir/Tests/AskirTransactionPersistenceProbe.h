#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AskirTransactionPersistenceProbe.generated.h"

class AAskirGameMode;
class UAskirSaveGame;

/** Reflected receiver needed to exercise the real dynamic inventory delegate. */
UCLASS()
class UAskirTransactionPersistenceProbe : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() TObjectPtr<AAskirGameMode> Mode;
    UPROPERTY() TObjectPtr<UAskirSaveGame> Snapshot;
    bool bCallbackObserved=false;
    bool bCaptureBlocked=false;
    bool bRestoreBlocked=false;
    UFUNCTION() void ObserveInventoryChange();
    UFUNCTION() void ObserveReputationChange(FName Faction,int32 NewReputation);
};
