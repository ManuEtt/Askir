#pragma once
#include "CoreMinimal.h"
class UAskirSaveGame;

/** Alternating validated generations, with read-only compatibility for V76 slots. */
namespace AskirSaveSlots
{
 ASKIR_API UAskirSaveGame* ReadLatest(const FString& SlotName);
 ASKIR_API bool WriteNext(const FString& SlotName,UAskirSaveGame* Snapshot);
}
