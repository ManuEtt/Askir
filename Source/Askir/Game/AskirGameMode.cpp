#include "Game/AskirGameMode.h"
#include "Frontend/AskirFrontendSubsystem.h"
#include "Game/AskirPlayerController.h"
#include "Magic/AskirMagicComponent.h"
#include "Magic/AskirSpellProjectile.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Demo/AskirDemoInteractable.h"
#include "Save/AskirSaveGame.h"
#include "Save/AskirSaveSlotStore.h"
#include "Equipment/AskirEquipmentComponent.h"
#include "Combat/AskirCombatComponent.h"
#include "Combat/AskirGuardComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Quest/AskirQuestComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "Economy/AskirMerchantComponent.h"
#include "Economy/AskirMerchantStateRules.h"
#include "Simulation/AskirSimulationSubsystem.h"
#include "Simulation/AskirNeedsComponent.h"
#include "World/AskirVillageDirector.h"
#include "World/AskirWorldClockComponent.h"
#include "Social/AskirReputationComponent.h"
#include "Social/AskirFactionComponent.h"
#include "Social/AskirRelationshipComponent.h"
#include "NPC/AskirDialogueComponent.h"
#include "Mission/AskirDornfeldMissionComponent.h"
#include "Mission/AskirDornfeldMissionRelay.h"
#include "Mission/AskirDeliveryMissionComponent.h"
#include "Mission/AskirDeliveryBanditRelay.h"
#include "Mission/AskirDeliveryQuestActor.h"
#include "Loot/AskirLootPickup.h"
#include "Loot/AskirLootChest.h"
#include "Survival/AskirSurvivalComponent.h"
#include "Stealth/AskirStealthComponent.h"
#include "Morale/AskirMoraleComponent.h"
#include "Travel/AskirTravelComponent.h"
#include "Travel/AskirWorldTravelSubsystem.h"
#include "Camp/AskirCampComponent.h"
#include "Companion/AskirCompanionComponent.h"
#include "Exploration/AskirExplorationComponent.h"
#include "Crafting/AskirCraftingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
 UAskirWorldTravelSubsystem* TravelState(const AAskirGameMode* Mode)
 {
  UGameInstance* Instance=Mode?Mode->GetGameInstance():nullptr;
  return Instance?Instance->GetSubsystem<UAskirWorldTravelSubsystem>():nullptr;
 }
 FString ResolveSavedPackage(const UAskirSaveGame* Save)
 {
  if(!Save->SavedMapPackageName.IsEmpty())return Save->SavedMapPackageName;
  return Save->SavedMapName.IsNone()?FString():FString::Printf(TEXT("/Game/Askir/WorldV42/%s"),*Save->SavedMapName.ToString());
 }
 bool HasOpenTransactions(UWorld* World,const AAskirCharacter* Player)
 {
  if(Player && ((Player->InventoryComponent && Player->InventoryComponent->IsTransactionInProgress())
    || (Player->CraftingComponent && Player->CraftingComponent->IsTransactionInProgress())
    || (Player->QuestComponent && Player->QuestComponent->IsTransactionInProgress())
    || (Player->DeliveryMission && Player->DeliveryMission->IsTransactionInProgress())))return true;
  if(World)for(TActorIterator<AActor> It(World);It;++It)
  {
   TArray<UAskirMerchantComponent*> Components;It->GetComponents(Components);
   for(const auto* Merchant:Components)if(Merchant && Merchant->IsTransactionInProgress())return true;
  }
  return false;
 }
 bool CollectPersistentMerchants(UWorld* World,TArray<UAskirMerchantComponent*>& OutMerchants)
 {
  TSet<FName> Seen;
  for(TActorIterator<AActor> It(World);It;++It)
  {
   TArray<UAskirMerchantComponent*> Components;It->GetComponents(Components);
   for(auto* Merchant:Components)
   {
    if(!IsValid(Merchant) || Merchant->MerchantID.IsNone() || Seen.Contains(Merchant->MerchantID) || Merchant->IsTransactionInProgress())return false;
    Seen.Add(Merchant->MerchantID);OutMerchants.Add(Merchant);
   }
  }
  return static_cast<size_t>(OutMerchants.Num())<=AskirMerchantStateRules::MaxMerchantsPerMap;
 }
}

AAskirGameMode::AAskirGameMode()
{
 DefaultPawnClass=AAskirCharacter::StaticClass();
 PlayerControllerClass=AAskirPlayerController::StaticClass();
 PrimaryActorTick.bCanEverTick=true;
}

UAskirSaveGame* AAskirGameMode::CaptureSnapshot()
{
 AAskirCharacter* Player=Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 if(!Player || !GetWorld() || HasOpenTransactions(GetWorld(),Player))return nullptr;
 TArray<UAskirMerchantComponent*> Merchants;
 if(!CollectPersistentMerchants(GetWorld(),Merchants))return nullptr;
 UAskirSaveGame* Save=Cast<UAskirSaveGame>(UGameplayStatics::CreateSaveGameObject(UAskirSaveGame::StaticClass()));
 if(!Save)return nullptr;
 Save->SavedMapName=FName(*UGameplayStatics::GetCurrentLevelName(this,true));
 Save->SavedMapPackageName=GetWorld()->GetPackage()->GetName();
 const FString PIEPrefix=GetWorld()->StreamingLevelsPrefix;
 if(!PIEPrefix.IsEmpty())Save->SavedMapPackageName.ReplaceInline(*PIEPrefix,TEXT(""));
 Save->PlayerLocation=Player->GetActorLocation();
 Save->PlayerRotation=Player->GetActorRotation();
 Save->PlayerStamina=Player->Stamina;
 if(Player->CombatComponent)Save->PlayerHealth=Player->CombatComponent->GetHealth();
 if(Player->ProgressionComponent)Save->SavedProgression=Player->ProgressionComponent->CaptureProgress();
 if(Player->EquipmentComponent)
 {
  const auto* Gear=Player->EquipmentComponent.Get();
  Save->SavedWeaponID=Gear->EquippedWeaponID;Save->SavedArmorID=Gear->EquippedArmorID;
  Save->SavedWeaponRating=Gear->WeaponRating;Save->SavedArmorRating=Gear->ArmorRating;Save->SavedGearDurability=Gear->Durability;
 }
 TMap<UAskirItemData*,int64> InventoryTotals;
 if(Player->InventoryComponent && Player->InventoryComponent->Items.Num()>FMath::Clamp(Player->InventoryComponent->MaxInventorySlots,1,4096))return nullptr;
 if(Player->InventoryComponent)
  for(const FAskirInventoryEntry& Entry:Player->InventoryComponent->Items)
  {
   if(!IsValid(Entry.Item.Get()) || Entry.Quantity<=0 || Entry.Item->MaxStackSize<1 || Entry.Quantity>Entry.Item->MaxStackSize)return nullptr;
   int64& Total=InventoryTotals.FindOrAdd(Entry.Item.Get());Total+=Entry.Quantity;
   if(Total>MAX_int32)return nullptr;
   FAskirSavedInventoryEntry SavedEntry;SavedEntry.ItemAsset=FSoftObjectPath(Entry.Item.Get());SavedEntry.Quantity=Entry.Quantity;
   // Runtime-only items cannot be reconstructed in a packaged save.
   if(!SavedEntry.ItemAsset.ToString().StartsWith(TEXT("/Game/")))return nullptr;
   Save->SavedInventory.Add(SavedEntry);
  }
 if(Player->QuestComponent)
 {
  Save->SavedQuestStates=Player->QuestComponent->QuestStates;
  Save->SavedActiveObjectives=Player->QuestComponent->ActiveObjectives;
  Save->SavedObjectivesByQuest=Player->QuestComponent->ObjectivesByQuest;
  Save->SavedLegacyActiveQuestID=Player->QuestComponent->LegacyActiveQuestID;
  Save->SavedClaimedRewards=Player->QuestComponent->ClaimedRewards;
  Save->SavedConsumedQuestTriggers=Player->QuestComponent->ConsumedQuestTriggers;
  Save->SavedCompletedEncounters=Player->QuestComponent->CompletedEncounters;
 }
 if(Player->TradeComponent)Save->SavedPlayerGold=Player->TradeComponent->Gold;
 if(Player->DornfeldMission)Save->SavedDornfeldMission=Player->DornfeldMission->CaptureMission();
 if(Player->MagicComponent)Save->SavedMagic=Player->MagicComponent->CaptureSnapshot();
 FAskirPlayerSystemsSave& S=Save->SavedSystems;
 if(Player->SurvivalComponent){S.Warmth=Player->SurvivalComponent->Warmth;S.Wetness=Player->SurvivalComponent->Wetness;S.Exposure=Player->SurvivalComponent->Exposure;}
 if(Player->StealthComponent){S.Noise=Player->StealthComponent->Noise;S.Visibility=Player->StealthComponent->Visibility;S.Suspicion=Player->StealthComponent->Suspicion;}
 if(Player->MoraleComponent){S.Morale=Player->MoraleComponent->Morale;S.Fear=Player->MoraleComponent->Fear;S.Courage=Player->MoraleComponent->Courage;}
 if(Player->TravelComponent){S.TravelDistance=Player->TravelComponent->TravelDistance;S.TravelFatigue=Player->TravelComponent->TravelFatigue;S.Supplies=Player->TravelComponent->Supplies;}
 if(Player->CampComponent){S.Fuel=Player->CampComponent->Fuel;S.Food=Player->CampComponent->Food;S.Shelter=Player->CampComponent->Shelter;}
 if(Player->CompanionComponent){S.Loyalty=Player->CompanionComponent->Loyalty;S.Bond=Player->CompanionComponent->Bond;S.CompanionStress=Player->CompanionComponent->Stress;}
 if(Player->ExplorationComponent){S.DiscoveredSites=Player->ExplorationComponent->DiscoveredSites;S.LoreKnowledge=Player->ExplorationComponent->LoreKnowledge;S.SurveyProgress=Player->ExplorationComponent->SurveyProgress;}
 if(Player->CraftingComponent){S.Materials=Player->CraftingComponent->Materials;S.CraftedItems=Player->CraftingComponent->CraftedItems;S.CraftSkill=Player->CraftingComponent->CraftSkill;S.UnlockedRecipes=Player->CraftingComponent->UnlockedRecipes;}
 if(Player->TradeComponent){S.MerchantFavor=Player->TradeComponent->MerchantFavor;S.TradeVolume=Player->TradeComponent->TradeVolume;}
 if(Player->NeedsComponent)S.Needs=Player->NeedsComponent->Needs;
 if(Player->GuardComponent)S.GuardStamina=Player->GuardComponent->Stamina;
 if(auto* Faction=Player->FindComponentByClass<UAskirFactionComponent>()){S.Faction=Faction->Faction;S.FactionStandings=Faction->Standings;}
 if(auto* Relationship=Player->FindComponentByClass<UAskirRelationshipComponent>())S.Relationships=Relationship->Relationships;
 if(auto* Rep=Player->FindComponentByClass<UAskirReputationComponent>())
  for(const auto& Standing:Rep->Standings)Save->SavedFactionReputation.Add(Standing.Faction,Standing.Reputation);
 if(auto* Dialogue=Player->FindComponentByClass<UAskirDialogueComponent>())Save->SavedDialogueFlags=Dialogue->GetFlags();
 if(auto* Sim=GetWorld()->GetSubsystem<UAskirSimulationSubsystem>())Save->WorldState=Sim->GetWorldState();
 for(TActorIterator<AAskirVillageDirector> It(GetWorld());It;++It)
 {
  if(!It->Clock)continue;
  Save->SavedWorldDay=It->Clock->CurrentDay;Save->SavedWorldHour=It->Clock->CurrentHour;
  for(const auto& Event:It->Clock->TimedEvents)
  {
   if(Event.EventID.IsNone())continue;
   Save->SavedWorldEventLastDays.Add(Event.EventID,Event.LastTriggeredDay);
   if(!Event.bRepeatDaily && Event.LastTriggeredDay>=0)Save->CompletedOneShotWorldEvents.AddUnique(Event.EventID);
  }
  break;
 }
 Save->DemoStage=DemoStage;Save->DemoSupplies=DemoSupplies;Save->bDemoFinished=bDemoFinished;Save->bDemoEnemyDefeated=DemoEnemy && DemoEnemy->IsDead();
 if(const auto* State=TravelState(this))
 {
  Save->SavedLootByMap=State->LootByMap;
  Save->SavedMerchantsByMap=State->MerchantsByMap;
 }
 FAskirMapMerchantState& MerchantState=Save->SavedMerchantsByMap.FindOrAdd(Save->SavedMapName);
 // Replace the active map's ledger when rewinding or leaving this timeline.
 MerchantState.Merchants.Reset();
 for(auto* Merchant:Merchants)
 {
  FAskirMerchantSnapshot Snapshot;
  if(!Merchant->CaptureInitialState() || !Merchant->CaptureSnapshot(Snapshot))return nullptr;
  MerchantState.Merchants.Add(Merchant->MerchantID,MoveTemp(Snapshot));
 }
 FAskirLootState& LootState=Save->SavedLootByMap.FindOrAdd(Save->SavedMapName);
 // Replace this map's state, allowing an older load to rewind the active timeline.
 LootState=FAskirLootState();
 TSet<FName> SeenIDs;
 for(TActorIterator<AAskirLootPickup> It(GetWorld());It;++It)
 {
  if(It->PersistentLootID.IsNone())continue;
  if(SeenIDs.Contains(It->PersistentLootID)){UE_LOG(LogTemp,Error,TEXT("Duplicate persistent loot ID: %s"),*It->PersistentLootID.ToString());return nullptr;}
  SeenIDs.Add(It->PersistentLootID);
  if(It->IsCollected())LootState.CollectedPickups.Add(It->PersistentLootID);
 }
 for(TActorIterator<AAskirLootChest> It(GetWorld());It;++It)
 {
  if(It->PersistentLootID.IsNone())continue;
  if(SeenIDs.Contains(It->PersistentLootID)){UE_LOG(LogTemp,Error,TEXT("Duplicate persistent loot ID: %s"),*It->PersistentLootID.ToString());return nullptr;}
  SeenIDs.Add(It->PersistentLootID);
  if(It->IsOpened())LootState.OpenedChests.Add(It->PersistentLootID);
 }
 FString Reason;
 if(!Save->ValidateSnapshot(Reason)){UE_LOG(LogTemp,Warning,TEXT("Askir capture invalid: %s"),*Reason);return nullptr;}
 return Save;
}

bool AAskirGameMode::SaveGame(const FString& SlotName)
{
 if(const auto* Instance=GetGameInstance())if(const auto* Frontend=Instance->GetSubsystem<UAskirFrontendSubsystem>())if(Frontend->IsFreshStartPending())return false;
 if(bPersistenceOperation || !UAskirSaveGame::IsSafeSlotName(SlotName))return false;
 if(const auto* State=TravelState(this);State && State->HasPendingTravel())return false;
 TGuardValue<bool> OperationGuard(bPersistenceOperation,true);
 UAskirSaveGame* Save=CaptureSnapshot();
 if(!Save || !AskirSaveSlots::WriteNext(SlotName,Save))return false;
 if(auto* State=TravelState(this))
 {
  State->LootByMap=Save->SavedLootByMap;
  State->MerchantsByMap=Save->SavedMerchantsByMap;
 }
 return true;
}

bool AAskirGameMode::LoadGame(const FString& SlotName,bool bRestorePosition)
{
 if(const auto* Instance=GetGameInstance())if(const auto* Frontend=Instance->GetSubsystem<UAskirFrontendSubsystem>())if(Frontend->IsFreshStartPending())return false;
 if(bPersistenceOperation || !GetWorld() || !UAskirSaveGame::IsSafeSlotName(SlotName))return false;
 if(const auto* State=TravelState(this);State && State->HasPendingTravel())return false;
 if(HasOpenTransactions(GetWorld(),Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this,0))))return false;
 UAskirSaveGame* Save=AskirSaveSlots::ReadLatest(SlotName);
 if(!Save)return false;
 TStrongObjectPtr<UAskirSaveGame> SnapshotLifetime(Save);
 const FName CurrentMap(*UGameplayStatics::GetCurrentLevelName(this,true));
 if(bRestorePosition && !Save->SavedMapName.IsNone() && Save->SavedMapName!=CurrentMap)
 {
  const FString Package=ResolveSavedPackage(Save);
  auto* State=TravelState(this);
  if(!State || !UAskirWorldTravelSubsystem::IsUsableWorldPackage(Package,Save->SavedMapName))return false;
  for(const auto& Entry:Save->SavedInventory)if(!Cast<UAskirItemData>(Entry.ItemAsset.TryLoad()))return false;
  if(!State->QueueSnapshot(Save->SavedMapName,NAME_None,Save,true))return false;
  if(auto* Player=Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this,0)))
  {
   Player->SuspendForWorldTravel();
  }
  UGameplayStatics::OpenLevel(this,FName(*Package));
  return true; // Request accepted; arrival restore is asynchronous.
 }
 return RestoreSnapshot(Save,bRestorePosition);
}

bool AAskirGameMode::RestoreSnapshot(UAskirSaveGame* Save,bool bRestorePosition)
{
 if(bPersistenceOperation || !Save || !GetWorld())return false;
 TStrongObjectPtr<UAskirSaveGame> SnapshotLifetime(Save);
 AAskirCharacter* Player=Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 FString Reason;
 if(!Player || HasOpenTransactions(GetWorld(),Player) || !Save->ValidateSnapshot(Reason))return false;
 FAskirMagicSnapshot RestoredMagic=Save->SaveFormatVersion>=4?Save->SavedMagic:FAskirMagicSnapshot();
 if(Save->SaveFormatVersion<4&&Player->MagicComponent)RestoredMagic.Mana=Player->MagicComponent->MaxMana;
 if(Player->MagicComponent&&!Player->MagicComponent->CanRestoreSnapshot(RestoredMagic))return false;
 // All fallible validation and resource resolution precede live-state mutation.
 TArray<FAskirInventoryEntry> RestoredInventory;
 TMap<UAskirItemData*,int64> Totals;
 const int32 SlotLimit=Player->InventoryComponent?FMath::Clamp(Player->InventoryComponent->MaxInventorySlots,1,4096):4096;
 for(const auto& SavedEntry:Save->SavedInventory)
 {
  UAskirItemData* Item=Cast<UAskirItemData>(SavedEntry.ItemAsset.TryLoad());
  if(!Item || Item->MaxStackSize<1)return false;
  if(Save->SaveFormatVersion>=3 && SavedEntry.Quantity>Item->MaxStackSize)return false;
  int64& Total=Totals.FindOrAdd(Item);Total+=SavedEntry.Quantity;
  if(Total>MAX_int32)return false;
  const int64 NeededSlots=(static_cast<int64>(SavedEntry.Quantity)+Item->MaxStackSize-1)/Item->MaxStackSize;
  if(NeededSlots>SlotLimit-RestoredInventory.Num())return false;
  int32 Remaining=SavedEntry.Quantity;
  while(Remaining>0)
  {
   FAskirInventoryEntry Entry;Entry.Item=Item;Entry.Quantity=FMath::Min(Remaining,Item->MaxStackSize);
   RestoredInventory.Add(Entry);Remaining-=Entry.Quantity;
  }
 }
 TSet<FName> SeenIDs;
 for(TActorIterator<AAskirLootPickup> It(GetWorld());It;++It)
 {
  if(It->PersistentLootID.IsNone())continue;
  if(SeenIDs.Contains(It->PersistentLootID))return false;
  SeenIDs.Add(It->PersistentLootID);
 }
 for(TActorIterator<AAskirLootChest> It(GetWorld());It;++It)
 {
  if(It->PersistentLootID.IsNone())continue;
  if(SeenIDs.Contains(It->PersistentLootID))return false;
  SeenIDs.Add(It->PersistentLootID);
 }
 TArray<UAskirMerchantComponent*> Merchants;
 if(!CollectPersistentMerchants(GetWorld(),Merchants))return false;
 const FName CurrentMap(*UGameplayStatics::GetCurrentLevelName(this,true));
 const FAskirMapMerchantState* SavedMerchantMap=Save->SaveFormatVersion>=5?Save->SavedMerchantsByMap.Find(CurrentMap):nullptr;
 // A v5 save captured on this same map must contain every authored merchant.
 // Missing receipts may initialize a newly visited map during travel, but may
 // never reset traded stock while loading the map that was actually saved.
 const bool bRequireMerchantReceipts=Save->SaveFormatVersion>=5 && (Save->SavedMapName.IsNone() || Save->SavedMapName==CurrentMap);
 TArray<FAskirMerchantSnapshot> PreparedMerchants;
 for(auto* Merchant:Merchants)
 {
  const FAskirMerchantSnapshot* SavedMerchant=SavedMerchantMap?SavedMerchantMap->Merchants.Find(Merchant->MerchantID):nullptr;
  if(bRequireMerchantReceipts && !SavedMerchant)return false;
  FAskirMerchantSnapshot Prepared;
  if(AskirMerchantStateRules::RestoreInitialState(Save->SaveFormatVersion,SavedMerchant!=nullptr))
  {
   if(!Merchant->CaptureInitialState() || !Merchant->GetInitialSnapshot(Prepared))return false;
  }
  else Prepared=*SavedMerchant;
  if(!Merchant->CanRestoreSnapshot(Prepared))return false;
  PreparedMerchants.Add(MoveTemp(Prepared));
 }
 FVector RestoreLocation=Save->PlayerLocation;
 if(bRestorePosition && !GetWorld()->FindTeleportSpot(Player,RestoreLocation,Save->PlayerRotation))return false;
 TGuardValue<bool> OperationGuard(bPersistenceOperation,true);
 // The complete map set was validated first; no partially installed merchant
 // wallet can become visible to a player or inventory delegate.
 for(int32 Index=0;Index<Merchants.Num();++Index)Merchants[Index]->RestoreSnapshot(PreparedMerchants[Index]);
 // In-flight projectiles belong to the outgoing timeline, never a loaded save.
 for(TActorIterator<AAskirSpellProjectile> It(GetWorld());It;++It)It->Destroy();
 if(Player->MagicComponent)Player->MagicComponent->RestoreSnapshot(RestoredMagic);
 if(Player->InventoryComponent)Player->InventoryComponent->Items=MoveTemp(RestoredInventory);
 if(Player->QuestComponent)
 {
  Player->QuestComponent->QuestStates=Save->SavedQuestStates;
  Player->QuestComponent->ActiveObjectives=Save->SavedActiveObjectives;
  Player->QuestComponent->ObjectivesByQuest=Save->SavedObjectivesByQuest;
  Player->QuestComponent->LegacyActiveQuestID=Save->SavedLegacyActiveQuestID;
  Player->QuestComponent->ClaimedRewards=Save->SavedClaimedRewards;
  Player->QuestComponent->ConsumedQuestTriggers=Save->SavedConsumedQuestTriggers;
  Player->QuestComponent->CompletedEncounters=Save->SavedCompletedEncounters;
 }
 if(Player->TradeComponent)Player->TradeComponent->Gold=Save->SavedPlayerGold;
 if(Player->ProgressionComponent)Player->ProgressionComponent->Progress=Save->SavedProgression;
 if(Player->EquipmentComponent)
 {
  Player->EquipmentComponent->RestoreEquipment(Save->SavedWeaponID,Save->SavedArmorID,Save->SavedWeaponRating,Save->SavedArmorRating,Save->SavedGearDurability);
  Player->EquipmentComponent->ReconcileWithInventory();
 }
 // Older formats did not capture these: restore explicit defaults, never a mixture
 // with resources from the currently running timeline.
 const FAskirPlayerSystemsSave S=Save->SaveFormatVersion>=3?Save->SavedSystems:FAskirPlayerSystemsSave();
 if(Player->SurvivalComponent){Player->SurvivalComponent->Warmth=S.Warmth;Player->SurvivalComponent->Wetness=S.Wetness;Player->SurvivalComponent->Exposure=S.Exposure;}
 if(Player->StealthComponent){Player->StealthComponent->Noise=S.Noise;Player->StealthComponent->Visibility=S.Visibility;Player->StealthComponent->Suspicion=S.Suspicion;}
 if(Player->MoraleComponent){Player->MoraleComponent->Morale=S.Morale;Player->MoraleComponent->Fear=S.Fear;Player->MoraleComponent->Courage=S.Courage;}
 if(Player->TravelComponent){Player->TravelComponent->TravelDistance=S.TravelDistance;Player->TravelComponent->TravelFatigue=S.TravelFatigue;Player->TravelComponent->Supplies=S.Supplies;}
 if(Player->CampComponent){Player->CampComponent->Fuel=S.Fuel;Player->CampComponent->Food=S.Food;Player->CampComponent->Shelter=S.Shelter;}
 if(Player->CompanionComponent){Player->CompanionComponent->Loyalty=S.Loyalty;Player->CompanionComponent->Bond=S.Bond;Player->CompanionComponent->Stress=S.CompanionStress;}
 if(Player->ExplorationComponent){Player->ExplorationComponent->DiscoveredSites=S.DiscoveredSites;Player->ExplorationComponent->LoreKnowledge=S.LoreKnowledge;Player->ExplorationComponent->SurveyProgress=S.SurveyProgress;}
 if(Player->CraftingComponent){Player->CraftingComponent->Materials=S.Materials;Player->CraftingComponent->CraftedItems=S.CraftedItems;Player->CraftingComponent->CraftSkill=S.CraftSkill;Player->CraftingComponent->UnlockedRecipes=S.UnlockedRecipes;}
 if(Player->TradeComponent){Player->TradeComponent->MerchantFavor=S.MerchantFavor;Player->TradeComponent->TradeVolume=S.TradeVolume;}
 if(Player->NeedsComponent)Player->NeedsComponent->Needs=S.Needs;
 if(Player->GuardComponent){Player->GuardComponent->StopGuard();Player->GuardComponent->Stamina=FMath::Clamp(S.GuardStamina,0.f,Player->GuardComponent->MaxStamina);}
 if(auto* Faction=Player->FindComponentByClass<UAskirFactionComponent>()){Faction->Faction=S.Faction;Faction->Standings=S.FactionStandings;}
 if(auto* Relationship=Player->FindComponentByClass<UAskirRelationshipComponent>())Relationship->Relationships=S.Relationships;
 if(auto* Rep=Player->FindComponentByClass<UAskirReputationComponent>())
 {
  Rep->Standings.Reset();
  for(const auto& Pair:Save->SavedFactionReputation){FAskirFactionStanding Standing;Standing.Faction=Pair.Key;Standing.Reputation=Pair.Value;Rep->Standings.Add(Standing);}
 }
 if(auto* Dialogue=Player->FindComponentByClass<UAskirDialogueComponent>()){Dialogue->EndDialogue();Dialogue->RestoreFlags(Save->SavedDialogueFlags);}
 DemoStage=FMath::Clamp(Save->DemoStage,0,4);DemoSupplies=FMath::Max(0,Save->DemoSupplies);bDemoFinished=Save->bDemoFinished;
 if(DemoEnemy && DemoEnemy->CombatComponent)DemoEnemy->CombatComponent->RestoreHealth(Save->bDemoEnemyDefeated?0.f:DemoEnemy->CombatComponent->GetMaxHealth());
 for(AAskirDemoInteractable* Obj:DemoObjects)if(Obj)Obj->SetUsed(Obj->Kind==EAskirDemoObject::Supplies && DemoSupplies>0);
 if(auto* State=TravelState(this))
 {
  State->LootByMap=Save->SavedLootByMap;
  State->MerchantsByMap=Save->SaveFormatVersion>=5?Save->SavedMerchantsByMap:TMap<FName,FAskirMapMerchantState>();
 }
 const FAskirLootState* Loot=Save->SavedLootByMap.Find(CurrentMap);
 for(TActorIterator<AAskirLootPickup> It(GetWorld());It;++It)
  if(!It->PersistentLootID.IsNone())It->RestoreLootState(Loot && Loot->CollectedPickups.Contains(It->PersistentLootID));
 for(TActorIterator<AAskirLootChest> It(GetWorld());It;++It)
  if(!It->PersistentLootID.IsNone())It->RestoreLootState(Loot && Loot->OpenedChests.Contains(It->PersistentLootID));
 if(Player->QuestComponent)Player->QuestComponent->NormalizeRestoredState(false);
 if(Player->DornfeldMission)Player->DornfeldMission->RestoreMission(Save->SavedDornfeldMission,false);
 for(TActorIterator<AAskirDornfeldMissionRelay> It(GetWorld());It;++It)It->RefreshMissionState();
 if(Player->DeliveryMission)Player->DeliveryMission->RefreshFromQuestLedger();
 // Install encounter and cargo visibility quietly before the first restore
 // notification. Progression/inventory observers must see the loaded timeline.
 for(TActorIterator<AAskirDeliveryBanditRelay> It(GetWorld());It;++It)It->RefreshMissionState();
 for(TActorIterator<AAskirDeliveryQuestActor> It(GetWorld());It;++It)It->RefreshMissionState();
 if(Player->CombatComponent)Player->CombatComponent->RestoreHealth(Save->PlayerHealth);
 Player->Stamina=FMath::Clamp(Save->PlayerStamina,0.f,Player->MaxStamina);
 Player->RestoreAfterLoad();
 if(bRestorePosition)
 {
  Player->SetActorLocationAndRotation(RestoreLocation,Save->PlayerRotation,false,nullptr,ETeleportType::TeleportPhysics);
  if(auto* Controller=Cast<APlayerController>(Player->GetController()))Controller->SetControlRotation(Save->PlayerRotation);
 }
 // Notify only after inventory, resources and reward ledgers have been installed.
 // A menu belongs to the outgoing timeline. Invalidate its buttons before any
 // restore delegates can observe or act on the newly installed inventory.
 if(auto* Controller=Cast<AAskirPlayerController>(Player->GetController()))Controller->CloseAnyMenu();
 if(Player->ProgressionComponent)Player->ProgressionComponent->OnProgressionChanged.Broadcast();
 if(auto* Sim=GetWorld()->GetSubsystem<UAskirSimulationSubsystem>())Sim->RestoreWorldState(Save->WorldState);
 for(TActorIterator<AAskirVillageDirector> It(GetWorld());It;++It)
 {
  if(!It->Clock)continue;
  TMap<FName,int32> EventDays=Save->SavedWorldEventLastDays;
  if(Save->SaveFormatVersion<3)for(FName ID:Save->CompletedOneShotWorldEvents)EventDays.Add(ID,Save->SavedWorldDay);
  It->Clock->RestoreClock(Save->SavedWorldDay,Save->SavedWorldHour,EventDays);
  It->RefreshResidents();break;
 }
 if(Player->DornfeldMission)Player->DornfeldMission->NotifyMissionRestored();
 if(Player->QuestComponent)Player->QuestComponent->OnQuestProgressChanged.Broadcast(NAME_None,NAME_None);
 if(Player->InventoryComponent)Player->InventoryComponent->OnInventoryChanged.Broadcast();
 return true;
}
