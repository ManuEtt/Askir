#include "Mission/AskirDeliveryMissionComponent.h"
#include "Mission/AskirDeliveryRules.h"
#include "Characters/AskirCharacter.h"
#include "Game/AskirGameMode.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "NPC/AskirDialogueComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Quest/AskirQuestData.h"
#include "Social/AskirReputationComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"

namespace
{
const FName AcceptedReceipt(TEXT("SQ_001_ACCEPTED"));
const FName CargoReceipt(TEXT("SQ_001_CARGO_RECOVERED"));
const FName HelpFlag(TEXT("SQ_001_CHOICE_help"));
const FName BargainFlag(TEXT("SQ_001_CHOICE_bargain"));
const TCHAR* KillPrefix=TEXT("SQ_001_KILL_");
FName KillReceipt(FName ID){return ID.IsNone()?NAME_None:FName(FString(KillPrefix)+ID.ToString());}
FName CargoSourceReceipt(FName ID){return ID.IsNone()?NAME_None:FName(FString(TEXT("SQ_001_CARGO_SOURCE_"))+ID.ToString());}
using RulesStage=AskirDeliveryRules::Stage;
static_assert(uint8(EAskirDeliveryStage::Completed)==uint8(RulesStage::Completed),"Delivery stage mapping changed");
RulesStage ToRules(EAskirDeliveryStage Stage){return static_cast<RulesStage>(Stage);}
FAskirQuestObjective Objective(const TCHAR* ID,const FText& Description,int32 Required,int32 Current)
{
    FAskirQuestObjective Result;Result.ObjectiveID=FName(ID);Result.Description=Description;
    Result.RequiredCount=Required;Result.CurrentCount=Current;return Result;
}
TArray<FAskirQuestObjective> DeliveryObjectives(int32 Kills,bool bCargo,bool bReturned)
{
    return {
        Objective(TEXT("SQ_001_A"),NSLOCTEXT("AskirDelivery","Kill","Besiege die beiden Wegelagerer."),2,Kills),
        Objective(TEXT("SQ_001_CARGO"),NSLOCTEXT("AskirDelivery","Recover","Sichere Torbens Lieferung im Lager."),1,bCargo?1:0),
        Objective(TEXT("SQ_001_B"),NSLOCTEXT("AskirDelivery","Return","Kehre zu Torben zurück."),1,bReturned?1:0)};
}
}

UAskirDeliveryMissionComponent::UAskirDeliveryMissionComponent(){PrimaryComponentTick.bCanEverTick=false;}
void UAskirDeliveryMissionComponent::BeginPlay()
{
    Super::BeginPlay();
    if(UAskirQuestComponent* Journal=GetJournal())Journal->OnQuestProgressChanged.AddUniqueDynamic(this,&UAskirDeliveryMissionComponent::OnQuestProgressChanged);
    RefreshFromQuestLedger();
}
void UAskirDeliveryMissionComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if(UAskirQuestComponent* Journal=GetJournal())Journal->OnQuestProgressChanged.RemoveDynamic(this,&UAskirDeliveryMissionComponent::OnQuestProgressChanged);
    Super::EndPlay(Reason);
}
void UAskirDeliveryMissionComponent::OnQuestProgressChanged(FName QuestID,FName ObjectiveID)
{if(QuestID.IsNone() || QuestID==GetQuestID())RefreshFromQuestLedger();}
UAskirQuestComponent* UAskirDeliveryMissionComponent::GetJournal() const
{return GetOwner()?GetOwner()->FindComponentByClass<UAskirQuestComponent>():nullptr;}
bool UAskirDeliveryMissionComponent::CanMutate() const
{
    if(bTransactionInProgress || !IsValid(GetOwner()))return false;
    const AAskirCharacter* Character=Cast<AAskirCharacter>(GetOwner());
    if(!IsValid(Character) || !Character->IsPlayerControlled() || Character->IsDead() || Character->IsWorldTravelPending())return false;
    if(const AAskirGameMode* Mode=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))if(Mode->IsPersistenceBusy())return false;
    const UAskirQuestComponent* Journal=GetJournal();
    if(!Journal || Journal->IsTransactionInProgress())return false;
    const UAskirInventoryComponent* Bag=GetOwner()->FindComponentByClass<UAskirInventoryComponent>();
    return !Bag || !Bag->IsTransactionInProgress();
}
bool UAskirDeliveryMissionComponent::Fail(const FText& Reason){LastFailureText=Reason;return false;}
UAskirQuestData* UAskirDeliveryMissionComponent::ResolveQuestData()
{
    if(!QuestData)
    {
        QuestData=NewObject<UAskirQuestData>(this);
        QuestData->QuestID=GetQuestID();
        QuestData->Title=NSLOCTEXT("AskirDelivery","Title","Die verlorene Lieferung");
        QuestData->Description=NSLOCTEXT("AskirDelivery","Description","Wegelagerer haben Torbens Waren ins Dornfelder Umland gebracht. Besiege sie, sichere die Lieferung und kehre zu Torben zurück.");
        QuestData->Objectives=DeliveryObjectives(0,false,false);
        QuestData->GoldReward=AskirDeliveryRules::GoldReward;
        QuestData->ExperienceReward=AskirDeliveryRules::ExperienceReward;
        QuestData->RewardItemQuantity=AskirDeliveryRules::ItemRewardQuantity;
    }
    // Avoid a failed LoadObject for ungenerated maps. Retry when an editor import
    // makes the real asset available; never fabricate a replacement inventory item.
    if(!QuestData->RewardItem && FPackageName::DoesPackageExist(TEXT("/Game/Askir/V80/Items/item_002")))
        QuestData->RewardItem=LoadObject<UAskirItemData>(nullptr,TEXT("/Game/Askir/V80/Items/item_002.item_002"));
    if(QuestData->QuestID!=GetQuestID() || QuestData->GoldReward!=AskirDeliveryRules::GoldReward
        || QuestData->ExperienceReward!=AskirDeliveryRules::ExperienceReward || QuestData->RewardItemQuantity!=AskirDeliveryRules::ItemRewardQuantity)
        return nullptr;
    if(QuestData->RewardItem && QuestData->RewardItem->ItemID!=FName(TEXT("item_002")))return nullptr;
    // The persisted runtime objectives are canonical even if a legacy authored
    // asset still has only kill+talk. No asset is modified by this compatibility.
    return QuestData;
}
int32 UAskirDeliveryMissionComponent::GetBanditCount() const
{
    const UAskirQuestComponent* Journal=GetJournal();if(!Journal)return 0;
    int32 Count=0;
    for(FName Receipt:Journal->ConsumedQuestTriggers)
        if(Receipt.ToString().StartsWith(KillPrefix) && Receipt.ToString().Len()>FCString::Strlen(KillPrefix))
            if(++Count>=AskirDeliveryRules::RequiredBandits)break;
    return Count;
}
bool UAskirDeliveryMissionComponent::HasCargoReceipt() const
{const UAskirQuestComponent* Journal=GetJournal();return Journal && Journal->HasConsumedQuestTrigger(CargoReceipt);}
bool UAskirDeliveryMissionComponent::HasCreditedBandit(FName ID) const
{const UAskirQuestComponent* Journal=GetJournal();return Journal && !ID.IsNone() && Journal->HasConsumedQuestTrigger(KillReceipt(ID));}
bool UAskirDeliveryMissionComponent::HasRecoveredCargo(FName ID) const
{const UAskirQuestComponent* Journal=GetJournal();return Journal && !ID.IsNone() && HasCargoReceipt() && Journal->HasConsumedQuestTrigger(CargoSourceReceipt(ID));}
EAskirDeliveryStage UAskirDeliveryMissionComponent::GetStage() const
{
    const UAskirQuestComponent* Journal=GetJournal();if(!Journal)return EAskirDeliveryStage::NotStarted;
    return static_cast<EAskirDeliveryStage>(AskirDeliveryRules::ResolveStage(
        Journal->GetStateByID(GetQuestID())==EAskirQuestState::Active,
        Journal->GetStateByID(GetQuestID())==EAskirQuestState::Completed,
        Journal->HasClaimedReward(GetQuestID()),GetBanditCount(),HasCargoReceipt()));
}
FName UAskirDeliveryMissionComponent::GetAcceptedChoice() const
{
    const UAskirDialogueComponent* Dialogue=GetOwner()?GetOwner()->FindComponentByClass<UAskirDialogueComponent>():nullptr;
    if(Dialogue && Dialogue->HasFlag(HelpFlag))return FName(TEXT("help"));
    if(Dialogue && Dialogue->HasFlag(BargainFlag))return FName(TEXT("bargain"));
    return NAME_None;
}
bool UAskirDeliveryMissionComponent::SyncObjectives(bool bReturned)
{
    UAskirQuestComponent* Journal=GetJournal();
    return Journal && Journal->RestoreQuestProgress(GetQuestID(),DeliveryObjectives(GetBanditCount(),HasCargoReceipt(),bReturned),EAskirQuestState::Active,false,false);
}
void UAskirDeliveryMissionComponent::RefreshFromQuestLedger()
{
    // The objective list is a journal mirror. Unique kill/cargo receipts in the
    // same generic ledger remain authoritative when loading legacy placeholders.
    UAskirQuestComponent* Journal=GetJournal();
    if(!bTransactionInProgress && Journal && Journal->GetStateByID(GetQuestID())==EAskirQuestState::Active)SyncObjectives();
}
bool UAskirDeliveryMissionComponent::AcceptDelivery(FName ChoiceID)
{
    UAskirQuestComponent* Journal=GetJournal();
    const bool bHelp=ChoiceID==FName(TEXT("help")),bBargain=ChoiceID==FName(TEXT("bargain"));
    if(!AskirDeliveryRules::CanAccept(ToRules(GetStage()),bHelp||bBargain,Journal && Journal->HasConsumedQuestTrigger(AcceptedReceipt),CanMutate()))
        return Fail(NSLOCTEXT("AskirDelivery","CannotAccept","Dieser Auftrag kann gerade nicht angenommen werden."));
    UAskirDialogueComponent* Dialogue=GetOwner()->FindComponentByClass<UAskirDialogueComponent>();
    UAskirReputationComponent* Reputation=GetOwner()->FindComponentByClass<UAskirReputationComponent>();
    UAskirQuestData* Data=ResolveQuestData();
    if(!Dialogue || !Reputation || !Data)return Fail(NSLOCTEXT("AskirDelivery","MissingSystems","Die benötigten Questdaten oder Spielersysteme fehlen."));
    TGuardValue<bool> Guard(bTransactionInProgress,true);
    if(!Journal->StartQuest(Data))return Fail(NSLOCTEXT("AskirDelivery","CannotStart","Der Auftrag konnte nicht begonnen werden."));
    // Receipt and choice precede the reputation delegate. Saving/reloading is
    // blocked for this transaction, including re-entry from a faction listener.
    Journal->ConsumedQuestTriggers.Add(AcceptedReceipt);
    const bool bChoiceAlreadyRecorded=Dialogue->HasFlag(HelpFlag)||Dialogue->HasFlag(BargainFlag);
    if(!bChoiceAlreadyRecorded)
    {
        Dialogue->AddFlag(bHelp?HelpFlag:BargainFlag);
        Reputation->ChangeReputation(bHelp?FName(TEXT("Waldhüter")):FName(TEXT("Freie Händler")),2);
    }
    SyncObjectives();
    LastFailureText=FText::GetEmpty();
    Journal->OnQuestProgressChanged.Broadcast(GetQuestID(),NAME_None);
    return true;
}
bool UAskirDeliveryMissionComponent::CreditBandit(FName ID)
{
    UAskirQuestComponent* Journal=GetJournal();
    if(!AskirDeliveryRules::CanCreditBandit(ToRules(GetStage()),!ID.IsNone(),HasCreditedBandit(ID),GetBanditCount(),true,CanMutate()))return false;
    TGuardValue<bool> Guard(bTransactionInProgress,true);
    Journal->ConsumedQuestTriggers.Add(KillReceipt(ID));
    if(!SyncObjectives()){Journal->ConsumedQuestTriggers.Remove(KillReceipt(ID));return false;}
    Journal->OnQuestProgressChanged.Broadcast(GetQuestID(),FName(TEXT("SQ_001_A")));
    return true;
}
bool UAskirDeliveryMissionComponent::RecoverCargo(FName ID)
{
    UAskirQuestComponent* Journal=GetJournal();
    if(!AskirDeliveryRules::CanRecoverCargo(ToRules(GetStage()),!ID.IsNone(),HasRecoveredCargo(ID),CanMutate()))
        return Fail(NSLOCTEXT("AskirDelivery","CargoBlocked","Besiege zuerst beide Wegelagerer, um die Lieferung zu sichern."));
    TGuardValue<bool> Guard(bTransactionInProgress,true);
    Journal->ConsumedQuestTriggers.Add(CargoReceipt);
    Journal->ConsumedQuestTriggers.Add(CargoSourceReceipt(ID));
    if(!SyncObjectives())
    {
        Journal->ConsumedQuestTriggers.Remove(CargoReceipt);Journal->ConsumedQuestTriggers.Remove(CargoSourceReceipt(ID));return false;
    }
    LastFailureText=FText::GetEmpty();
    Journal->OnQuestProgressChanged.Broadcast(GetQuestID(),FName(TEXT("SQ_001_CARGO")));
    return true;
}
bool UAskirDeliveryMissionComponent::ReturnToTorben()
{
    UAskirQuestComponent* Journal=GetJournal();
    UAskirQuestData* Data=ResolveQuestData();
    AActor* Owner=GetOwner();
    UAskirInventoryComponent* Bag=Owner?Owner->FindComponentByClass<UAskirInventoryComponent>():nullptr;
    UAskirTradeComponent* Wallet=Owner?Owner->FindComponentByClass<UAskirTradeComponent>():nullptr;
    UAskirProgressionComponent* Progression=Owner?Owner->FindComponentByClass<UAskirProgressionComponent>():nullptr;
    if(!AskirDeliveryRules::CanTurnIn(ToRules(GetStage()),Journal && Journal->HasClaimedReward(GetQuestID()),CanMutate(),Data && IsValid(Data->RewardItem),IsValid(Bag),IsValid(Wallet),IsValid(Progression),Wallet?Wallet->Gold:0))
        return Fail(NSLOCTEXT("AskirDelivery","CannotReturn","Die Lieferung oder gültige Belohnungsdaten fehlen; ein erneuter Abschluss ist ausgeschlossen."));
    TGuardValue<bool> Guard(bTransactionInProgress,true);
    const TArray<FAskirQuestObjective> PreviousObjectives=Journal->GetObjectivesForQuest(GetQuestID());
    if(!SyncObjectives(true))
        return Fail(NSLOCTEXT("AskirDelivery","FullInventory","Die Belohnung passt nicht ins Inventar oder die Auszahlung ist gerade gesperrt."));
    if(!Journal->TurnInQuestWithRewards(Data))
    {
        // A full bag or invalid XP must leave the return objective untouched.
        // The quest commit has no callbacks on failure, so this restores the
        // exact prior objective list before the transaction becomes observable.
        Journal->RestoreQuestProgress(GetQuestID(),PreviousObjectives,EAskirQuestState::Active,false,false);
        return Fail(NSLOCTEXT("AskirDelivery","FullInventory","Die Belohnung passt nicht ins Inventar oder die Auszahlung ist gerade gesperrt."));
    }
    LastFailureText=FText::GetEmpty();
    Journal->OnQuestProgressChanged.Broadcast(GetQuestID(),NAME_None);
    return true;
}
FText UAskirDeliveryMissionComponent::GetObjectiveText() const
{
    switch(GetStage())
    {
    case EAskirDeliveryStage::NotStarted:return NSLOCTEXT("AskirDelivery","Start","Sprich mit Torben über seine verlorene Lieferung.");
    case EAskirDeliveryStage::DefeatBandits:return FText::Format(NSLOCTEXT("AskirDelivery","Bandits","Besiege die Wegelagerer im Dornfelder Umland ({0}/2)."),FText::AsNumber(GetBanditCount()));
    case EAskirDeliveryStage::RecoverCargo:return NSLOCTEXT("AskirDelivery","Cargo","Sichere Torbens Lieferung im Lager der Wegelagerer.");
    case EAskirDeliveryStage::ReturnToTorben:return NSLOCTEXT("AskirDelivery","Torben","Kehre mit der Lieferung zu Torben in Dornfeld zurück.");
    default:return NSLOCTEXT("AskirDelivery","Done","Torbens Lieferung ist gerettet.");
    }
}
