#include "Save/AskirSaveSlotStore.h"
#include "Save/AskirSaveGame.h"
#include "Kismet/GameplayStatics.h"

namespace AskirSaveSlots
{
 UAskirSaveGame* ReadLatest(const FString& SlotName)
 {
  if(!UAskirSaveGame::IsSafeSlotName(SlotName))return nullptr;
  UAskirSaveGame* Latest=nullptr;
  const FString Candidates[]={SlotName+TEXT("_BankA"),SlotName+TEXT("_BankB"),SlotName};
  for(const FString& Candidate:Candidates)
  {
   if(!UGameplayStatics::DoesSaveGameExist(Candidate,0))continue;
   UAskirSaveGame* Save=Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromSlot(Candidate,0));
   FString Reason;
   if(!Save || !Save->ValidateSnapshot(Reason))
   {
    UE_LOG(LogTemp,Warning,TEXT("Askir rejected save bank %s: %s"),*Candidate,*Reason);
    continue;
   }
   if(!Latest || Save->SaveGeneration>Latest->SaveGeneration)Latest=Save;
  }
  return Latest;
 }

 bool WriteNext(const FString& SlotName,UAskirSaveGame* Snapshot)
 {
  if(!Snapshot || !UAskirSaveGame::IsSafeSlotName(SlotName))return false;
  UAskirSaveGame* Previous=ReadLatest(SlotName);
  const int64 Generation=Previous?Previous->SaveGeneration:0;
  if(Generation==MAX_int64)return false;
  Snapshot->SaveGeneration=Generation+1;
  FString Reason;
  if(!Snapshot->ValidateSnapshot(Reason))
  {
   UE_LOG(LogTemp,Warning,TEXT("Askir refused invalid snapshot: %s"),*Reason);
   return false;
  }
  const FString Bank=SlotName+(Snapshot->SaveGeneration%2==0?TEXT("_BankA"):TEXT("_BankB"));
  // Only the inactive bank is overwritten. The preceding generation survives a
  // failed write or incomplete file; readers reject corrupt banks before selection.
  if(!UGameplayStatics::SaveGameToSlot(Snapshot,Bank,0))return false;
  UAskirSaveGame* Verified=Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromSlot(Bank,0));
  return Verified && Verified->SaveGeneration==Snapshot->SaveGeneration && Verified->ValidateSnapshot(Reason);
 }
}
