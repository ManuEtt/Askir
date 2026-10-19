#include "Tests/AskirTransactionPersistenceProbe.h"
#include "Game/AskirGameMode.h"
#include "Save/AskirSaveGame.h"

void UAskirTransactionPersistenceProbe::ObserveInventoryChange()
{
    bCallbackObserved=true;
    bCaptureBlocked=Mode && Mode->CaptureSnapshot()==nullptr;
    bRestoreBlocked=Mode && Snapshot && !Mode->RestoreSnapshot(Snapshot,false);
}

void UAskirTransactionPersistenceProbe::ObserveReputationChange(FName Faction,int32 NewReputation)
{
    ObserveInventoryChange();
}
