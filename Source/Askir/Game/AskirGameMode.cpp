#include "Mission/AskirDornfeldMissionComponent.h"
#include "Demo/AskirDemoInteractable.h"
#include "Characters/AskirNPC.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Loot/AskirLootPickup.h"
#include "Loot/AskirLootChest.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "Game/AskirGameMode.h"
#include "Progression/AskirProgressionComponent.h"
#include "Characters/AskirCharacter.h"
#include "Save/AskirSaveGame.h"
#include "Equipment/AskirEquipmentComponent.h"
#include "Combat/AskirCombatComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Quest/AskirQuestComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "Simulation/AskirSimulationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "World/AskirVillageDirector.h"
#include "Social/AskirReputationComponent.h"
#include "NPC/AskirDialogueComponent.h"

AAskirGameMode::AAskirGameMode()
{
    DefaultPawnClass = AAskirCharacter::StaticClass();
    PrimaryActorTick.bCanEverTick=true;
}

bool AAskirGameMode::SaveGame(const FString& SlotName)
{
    if (SlotName.IsEmpty() || !GetWorld()) return false;
    AAskirCharacter* Player = Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player) return false;

    UAskirSaveGame* Save = Cast<UAskirSaveGame>(UGameplayStatics::CreateSaveGameObject(UAskirSaveGame::StaticClass()));
    if (!Save) return false;
    Save->SavedMapName = FName(*UGameplayStatics::GetCurrentLevelName(this, true));
    if (const UAskirEquipmentComponent* Gear = Player->FindComponentByClass<UAskirEquipmentComponent>())
    {
        // Save only inventory-backed equipment IDs (also sanitizes stale loadouts).
        const_cast<UAskirEquipmentComponent*>(Gear)->ReconcileWithInventory();
        Save->SavedWeaponID = Gear->EquippedWeaponID;
        Save->SavedArmorID = Gear->EquippedArmorID;
        Save->SavedWeaponRating = Gear->WeaponRating;
        Save->SavedArmorRating = Gear->ArmorRating;
        Save->SavedGearDurability = Gear->Durability;
    }

    if (Player->ProgressionComponent) Save->SavedProgression = Player->ProgressionComponent->CaptureProgress();
    Save->DemoStage=DemoStage;
    Save->DemoSupplies=DemoSupplies;
    Save->bDemoFinished=bDemoFinished;
    Save->bDemoEnemyDefeated=DemoEnemy && DemoEnemy->IsDead();
    Save->PlayerLocation = Player->GetActorLocation();
    Save->PlayerRotation = Player->GetActorRotation();
    Save->PlayerStamina = Player->Stamina;
    if (Player->CombatComponent) Save->PlayerHealth = Player->CombatComponent->GetHealth();
    if (UAskirSimulationSubsystem* Sim = GetWorld()->GetSubsystem<UAskirSimulationSubsystem>())
        Save->WorldState = Sim->GetWorldState();

    if (Player->InventoryComponent)
    {
        for (const FAskirInventoryEntry& Entry : Player->InventoryComponent->Items)
        {
            if (!Entry.Item || Entry.Quantity <= 0) continue;
            FAskirSavedInventoryEntry SavedEntry;
            SavedEntry.ItemAsset = FSoftObjectPath(Entry.Item.Get());
            SavedEntry.Quantity = Entry.Quantity;
            Save->SavedInventory.Add(SavedEntry);
        }
    }
    if (Player->QuestComponent)
    {
        Save->SavedQuestStates = Player->QuestComponent->QuestStates;
        Save->SavedActiveObjectives = Player->QuestComponent->ActiveObjectives;
        Save->SavedObjectivesByQuest = Player->QuestComponent->ObjectivesByQuest;
        Save->SavedLegacyActiveQuestID = Player->QuestComponent->LegacyActiveQuestID;
        Save->SavedClaimedRewards = Player->QuestComponent->ClaimedRewards;
        Save->SavedCompletedEncounters = Player->QuestComponent->CompletedEncounters;
    }
    if (Player->TradeComponent) Save->SavedPlayerGold=Player->TradeComponent->Gold;
    if (Player->DornfeldMission) Save->SavedDornfeldMission=Player->DornfeldMission->CaptureMission();
    if (auto* Rep=Player->FindComponentByClass<UAskirReputationComponent>())
        for (const auto& Standing:Rep->Standings)Save->SavedFactionReputation.Add(Standing.Faction,Standing.Reputation);
    if (auto* Dialogue=Player->FindComponentByClass<UAskirDialogueComponent>())
        Save->SavedDialogueFlags=Dialogue->GetFlags();
    for (TActorIterator<AAskirVillageDirector> It(GetWorld());It;++It)
    {
        Save->SavedWorldDay=It->Clock->CurrentDay;
        Save->SavedWorldHour=It->Clock->CurrentHour;
        for(const auto& Event:It->Clock->TimedEvents)
            if(!Event.bRepeatDaily && Event.LastTriggeredDay>=0)
                Save->CompletedOneShotWorldEvents.AddUnique(Event.EventID);
        break;
    }
    // Carry previously recorded map loot forward instead of replacing the whole map ledger.
    if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        if (const UAskirSaveGame* Previous = Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
            Save->SavedLootByMap = Previous->SavedLootByMap;
    }
    // Stable IDs are assigned per placed actor in the level editor. Missing IDs are ignored.
    const FName MapKey(*UGameplayStatics::GetCurrentLevelName(this,true));
    FAskirLootState& LootState=Save->SavedLootByMap.FindOrAdd(MapKey);
    TSet<FName> SeenIDs;
    for(TActorIterator<AAskirLootPickup> It(GetWorld());It;++It)
    {
        if(It->PersistentLootID.IsNone())continue;
        if(SeenIDs.Contains(It->PersistentLootID)) {UE_LOG(LogTemp,Warning,TEXT("Duplicate persistent loot ID: %s"),*It->PersistentLootID.ToString());continue;}
        SeenIDs.Add(It->PersistentLootID);
        if(It->IsCollected())LootState.CollectedPickups.Add(It->PersistentLootID);
    }
    for(TActorIterator<AAskirLootChest> It(GetWorld());It;++It)
    {
        if(It->PersistentLootID.IsNone())continue;
        if(SeenIDs.Contains(It->PersistentLootID)) {UE_LOG(LogTemp,Warning,TEXT("Duplicate persistent loot ID: %s"),*It->PersistentLootID.ToString());continue;}
        SeenIDs.Add(It->PersistentLootID);
        if(It->IsOpened())LootState.OpenedChests.Add(It->PersistentLootID);
    }
    return UGameplayStatics::SaveGameToSlot(Save, SlotName, 0);
}

bool AAskirGameMode::LoadGame(const FString& SlotName, bool bRestorePosition)
{
    if (SlotName.IsEmpty() || !GetWorld() || !UGameplayStatics::DoesSaveGameExist(SlotName, 0)) return false;
    UAskirSaveGame* Save = Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    AAskirCharacter* Player = Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Save || !Player) return false;
    if (Save->SaveFormatVersion < 1 || Save->SaveFormatVersion > 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Askir unsupported save version: %d"), Save->SaveFormatVersion);
        return false;
    }

    // Resolve item assets before modifying the current game state. Missing assets
    // abort loading instead of silently discarding the player's inventory.
    TArray<FAskirInventoryEntry> RestoredInventory;
    for (const FAskirSavedInventoryEntry& SavedEntry : Save->SavedInventory)
    {
        if (SavedEntry.Quantity <= 0 || SavedEntry.ItemAsset.IsNull()) return false;
        UAskirItemData* Item = Cast<UAskirItemData>(SavedEntry.ItemAsset.TryLoad());
        if (!Item) return false;
        FAskirInventoryEntry Entry;
        Entry.Item = Item;
        Entry.Quantity = SavedEntry.Quantity;
        RestoredInventory.Add(Entry);
    }

    // Reject malformed equipment data before changing any live game state.
    if (!FMath::IsFinite(Save->SavedWeaponRating) || !FMath::IsFinite(Save->SavedArmorRating)
        || !FMath::IsFinite(Save->SavedGearDurability)
        || Save->SavedWeaponRating < 0.f || Save->SavedArmorRating < 0.f
        || Save->SavedGearDurability < 0.f || Save->SavedGearDurability > 100.f) return false;
    DemoStage=FMath::Clamp(Save->DemoStage,0,4);
    DemoSupplies=FMath::Max(0,Save->DemoSupplies);
    bDemoFinished=Save->bDemoFinished;
    if (DemoEnemy && Save->bDemoEnemyDefeated && !DemoEnemy->IsDead() && DemoEnemy->CombatComponent)
        DemoEnemy->CombatComponent->Kill();
    for(AAskirDemoInteractable* Obj:DemoObjects)
        if(Obj) Obj->SetUsed((Obj->Kind==EAskirDemoObject::Supplies && DemoSupplies>0));
    if (Player->ProgressionComponent) Player->ProgressionComponent->RestoreProgress(Save->SavedProgression);
    if (bRestorePosition) Player->SetActorLocationAndRotation(Save->PlayerLocation, Save->PlayerRotation);
    Player->Stamina = FMath::Clamp(Save->PlayerStamina, 0.f, Player->MaxStamina);
    if (Player->CombatComponent) Player->CombatComponent->RestoreHealth(Save->PlayerHealth);
    if (Player->InventoryComponent)
    {
        Player->InventoryComponent->Items = MoveTemp(RestoredInventory);
        Player->InventoryComponent->OnInventoryChanged.Broadcast();
    }
    if (Player->QuestComponent)
    {
        Player->QuestComponent->QuestStates = Save->SavedQuestStates;
        Player->QuestComponent->ActiveObjectives = Save->SavedActiveObjectives;
        Player->QuestComponent->ObjectivesByQuest = Save->SavedObjectivesByQuest;
        Player->QuestComponent->LegacyActiveQuestID = Save->SavedLegacyActiveQuestID;
        Player->QuestComponent->ClaimedRewards = Save->SavedClaimedRewards;
        Player->QuestComponent->CompletedEncounters = Save->SavedCompletedEncounters;
    }
    if (UAskirEquipmentComponent* Gear = Player->FindComponentByClass<UAskirEquipmentComponent>())
    {
        Gear->RestoreEquipment(Save->SavedWeaponID,Save->SavedArmorID,Save->SavedWeaponRating,Save->SavedArmorRating,Save->SavedGearDurability);
        Gear->ReconcileWithInventory();
    }
    if (Player->TradeComponent) Player->TradeComponent->Gold=FMath::Max(0,Save->SavedPlayerGold);
    if (Player->DornfeldMission) Player->DornfeldMission->RestoreMission(Save->SavedDornfeldMission);
    if (UAskirSimulationSubsystem* Sim = GetWorld()->GetSubsystem<UAskirSimulationSubsystem>())
        Sim->RestoreWorldState(Save->WorldState);
    if (auto* Rep=Player->FindComponentByClass<UAskirReputationComponent>())
    {
        Rep->Standings.Reset();
        for(const auto& Pair:Save->SavedFactionReputation)Rep->SetReputation(Pair.Key,Pair.Value);
    }
    if (auto* Dialogue=Player->FindComponentByClass<UAskirDialogueComponent>())
        Dialogue->RestoreFlags(Save->SavedDialogueFlags);
    for (TActorIterator<AAskirVillageDirector> It(GetWorld());It;++It)
    {
        It->Clock->SetClock(Save->SavedWorldDay,Save->SavedWorldHour);
        for(auto& Event:It->Clock->TimedEvents)
            if(!Event.bRepeatDaily && Save->CompletedOneShotWorldEvents.Contains(Event.EventID))
                Event.LastTriggeredDay=FMath::Max(1,Save->SavedWorldDay);
        It->RefreshResidents();
        break;
    }
    const FName MapKey(*UGameplayStatics::GetCurrentLevelName(this,true));
    if(const FAskirLootState* LootState=Save->SavedLootByMap.Find(MapKey))
    {
        for(TActorIterator<AAskirLootPickup> It(GetWorld());It;++It)
            if(!It->PersistentLootID.IsNone())It->RestoreLootState(LootState->CollectedPickups.Contains(It->PersistentLootID));
        for(TActorIterator<AAskirLootChest> It(GetWorld());It;++It)
            if(!It->PersistentLootID.IsNone())It->RestoreLootState(LootState->OpenedChests.Contains(It->PersistentLootID));
    }
    return true;
}
