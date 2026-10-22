#include "Mission/AskirHerbMissionComponent.h"
#include "Mission/AskirHerbRules.h"
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
#include "Combat/AskirCombatComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"

namespace
{
const FName AcceptedReceipt(TEXT("SQ_003_ACCEPTED"));
const FName ExploredReceipt(TEXT("SQ_003_EXPLORED"));
const FName SourceReceipt(TEXT("SQ_003_EXPLORED_SOURCE_SQ003_Meadow_V84"));
const FName HelpFlag(TEXT("SQ_003_CHOICE_help")),BargainFlag(TEXT("SQ_003_CHOICE_bargain"));
using RulesStage=AskirHerbRules::Stage;
static_assert(uint8(EAskirHerbStage::Completed)==uint8(RulesStage::Completed),"Herb stage mapping changed");
RulesStage ToRules(EAskirHerbStage Stage){return static_cast<RulesStage>(Stage);}
FAskirQuestObjective Objective(const TCHAR* ID,const FText& Text,int32 Current)
{
    FAskirQuestObjective Result;Result.ObjectiveID=FName(ID);Result.Description=Text;
    Result.RequiredCount=1;Result.CurrentCount=Current;return Result;
}
TArray<FAskirQuestObjective> HerbObjectives(bool bExplored,bool bReturned)
{
    return {Objective(TEXT("SQ_003_A"),NSLOCTEXT("AskirHerbs","Explore","Untersuche das Heilkräutervorkommen im Dornfelder Umland."),bExplored?1:0),
        Objective(TEXT("SQ_003_B"),NSLOCTEXT("AskirHerbs","Report","Berichte Rurik in Dornfeld."),bReturned?1:0)};
}
}

UAskirHerbMissionComponent::UAskirHerbMissionComponent(){PrimaryComponentTick.bCanEverTick=false;}
void UAskirHerbMissionComponent::BeginPlay()
{
    Super::BeginPlay();
    if(auto* Journal=GetJournal())Journal->OnQuestProgressChanged.AddUniqueDynamic(this,&UAskirHerbMissionComponent::OnQuestProgressChanged);
    RefreshFromQuestLedger();
}
void UAskirHerbMissionComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if(auto* Journal=GetJournal())Journal->OnQuestProgressChanged.RemoveDynamic(this,&UAskirHerbMissionComponent::OnQuestProgressChanged);
    Super::EndPlay(Reason);
}
void UAskirHerbMissionComponent::OnQuestProgressChanged(FName QuestID,FName ObjectiveID)
{if(QuestID.IsNone()||QuestID==GetQuestID())RefreshFromQuestLedger();}
UAskirQuestComponent* UAskirHerbMissionComponent::GetJournal() const
{return IsValid(GetOwner())?GetOwner()->FindComponentByClass<UAskirQuestComponent>():nullptr;}
bool UAskirHerbMissionComponent::CanMutate() const
{
    const auto* Player=Cast<AAskirCharacter>(GetOwner());
    if(bTransactionInProgress||!IsValid(Player)||Player->IsActorBeingDestroyed()||!Player->IsPlayerControlled()||Player->IsDead()||
        Player->IsWorldTravelPending()||!GetWorld()||GetWorld()->bIsTearingDown||UGameplayStatics::IsGamePaused(this)||
        !Player->CombatComponent||!Player->CombatComponent->IsDamageEnabled())return false;
    if(const auto* Mode=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))if(Mode->IsPersistenceBusy())return false;
    const auto* Journal=GetJournal();
    const auto* Bag=Player->FindComponentByClass<UAskirInventoryComponent>();
    return Journal&&!Journal->IsTransactionInProgress()&&Bag&&!Bag->IsTransactionInProgress();
}
bool UAskirHerbMissionComponent::Fail(const FText& Reason){LastFailureText=Reason;return false;}
UAskirQuestData* UAskirHerbMissionComponent::ResolveQuestData()
{
    if(!QuestData)
    {
        QuestData=NewObject<UAskirQuestData>(this);QuestData->QuestID=GetQuestID();
        QuestData->Title=NSLOCTEXT("AskirHerbs","Title","Heilkräuter für Mara");
        QuestData->Description=NSLOCTEXT("AskirHerbs","Description","Rurik sucht ein Heilkräutervorkommen für Maras Vorräte. Untersuche die Wiese im Dornfelder Umland und berichte ihm, ob die Pflanzen dort gesund wachsen.");
        QuestData->Objectives=HerbObjectives(false,false);QuestData->GoldReward=AskirHerbRules::GoldReward;
        QuestData->ExperienceReward=AskirHerbRules::ExperienceReward;QuestData->RewardItemQuantity=AskirHerbRules::ItemRewardQuantity;
    }
    if(!QuestData->RewardItem&&FPackageName::DoesPackageExist(TEXT("/Game/Askir/V84/Items/item_004")))
        QuestData->RewardItem=LoadObject<UAskirItemData>(nullptr,TEXT("/Game/Askir/V84/Items/item_004.item_004"));
    if(QuestData->QuestID!=GetQuestID()||QuestData->GoldReward!=AskirHerbRules::GoldReward||
        QuestData->ExperienceReward!=AskirHerbRules::ExperienceReward||QuestData->RewardItemQuantity!=AskirHerbRules::ItemRewardQuantity||
        QuestData->Objectives.Num()!=2)return nullptr;
    const auto* Explore=QuestData->Objectives.FindByPredicate([](const auto& O){return O.ObjectiveID==FName(TEXT("SQ_003_A"));});
    const auto* Report=QuestData->Objectives.FindByPredicate([](const auto& O){return O.ObjectiveID==FName(TEXT("SQ_003_B"));});
    if(!Explore||!Report||Explore->RequiredCount!=1||Report->RequiredCount!=1)return nullptr;
    if(QuestData->RewardItem&&QuestData->RewardItem->ItemID!=FName(TEXT("item_004")))return nullptr;
    return QuestData;
}
bool UAskirHerbMissionComponent::HasExplorationReceipt() const
{const auto* Journal=GetJournal();return Journal&&Journal->HasConsumedQuestTrigger(ExploredReceipt)&&Journal->HasConsumedQuestTrigger(SourceReceipt);}
bool UAskirHerbMissionComponent::HasExploredHerbs(FName SourceID) const
{return SourceID==GetCanonicalSourceID()&&HasExplorationReceipt();}
EAskirHerbStage UAskirHerbMissionComponent::GetStage() const
{
    const auto* Journal=GetJournal();if(!Journal)return EAskirHerbStage::NotStarted;
    return static_cast<EAskirHerbStage>(AskirHerbRules::ResolveStage(Journal->GetStateByID(GetQuestID())==EAskirQuestState::Active,
        Journal->GetStateByID(GetQuestID())==EAskirQuestState::Completed,Journal->HasClaimedReward(GetQuestID()),HasExplorationReceipt()));
}
FName UAskirHerbMissionComponent::GetAcceptedChoice() const
{
    const auto* Flags=IsValid(GetOwner())?GetOwner()->FindComponentByClass<UAskirDialogueComponent>():nullptr;
    if(Flags&&Flags->HasFlag(HelpFlag))return FName(TEXT("help"));
    if(Flags&&Flags->HasFlag(BargainFlag))return FName(TEXT("bargain"));return NAME_None;
}
bool UAskirHerbMissionComponent::SyncObjectives(bool bReturned)
{auto* Journal=GetJournal();return Journal&&Journal->RestoreQuestProgress(GetQuestID(),HerbObjectives(HasExplorationReceipt(),bReturned),EAskirQuestState::Active,false,false);}
void UAskirHerbMissionComponent::RefreshFromQuestLedger()
{
    const auto* Journal=GetJournal();
    if(!bTransactionInProgress&&Journal&&Journal->GetStateByID(GetQuestID())==EAskirQuestState::Active)SyncObjectives();
}
void UAskirHerbMissionComponent::NotifyProgress(FName ObjectiveID)
{
    if(IsValid(GetOwner())&&!GetOwner()->IsActorBeingDestroyed())
        if(auto* Journal=GetJournal())Journal->OnQuestProgressChanged.Broadcast(GetQuestID(),ObjectiveID);
}
bool UAskirHerbMissionComponent::AcceptHerbs(FName ChoiceID)
{
    auto* Journal=GetJournal();const bool bHelp=ChoiceID==FName(TEXT("help")),bBargain=ChoiceID==FName(TEXT("bargain"));
    if(!AskirHerbRules::CanAccept(ToRules(GetStage()),bHelp||bBargain,Journal&&Journal->HasConsumedQuestTrigger(AcceptedReceipt),CanMutate()))
        return Fail(NSLOCTEXT("AskirHerbs","CannotAccept","Dieser Auftrag kann gerade nicht angenommen werden."));
    auto* Flags=GetOwner()->FindComponentByClass<UAskirDialogueComponent>();
    auto* Reputation=GetOwner()->FindComponentByClass<UAskirReputationComponent>();
    auto* Data=ResolveQuestData();
    if(!Flags||!Reputation||!Data)return Fail(NSLOCTEXT("AskirHerbs","MissingSystems","Die benötigten Questdaten oder Spielersysteme fehlen."));
    TGuardValue<bool> Guard(bTransactionInProgress,true);
    if(!Journal->StartQuest(Data))return Fail(NSLOCTEXT("AskirHerbs","CannotStart","Der Auftrag konnte nicht begonnen werden."));
    Journal->ConsumedQuestTriggers.Add(AcceptedReceipt);
    const bool bExistingChoice=Flags->HasFlag(HelpFlag)||Flags->HasFlag(BargainFlag);
    if(!bExistingChoice)Flags->AddFlag(bHelp?HelpFlag:BargainFlag);
    SyncObjectives();LastFailureText=FText::GetEmpty();
    // Every quest/choice receipt precedes external faction and quest callbacks.
    if(!bExistingChoice)Reputation->ChangeReputation(bHelp?FName(TEXT("Waldhüter")):FName(TEXT("Freie Händler")),2);
    NotifyProgress();return true;
}
bool UAskirHerbMissionComponent::ExploreHerbs(FName SourceID)
{
    if(!AskirHerbRules::CanExplore(ToRules(GetStage()),SourceID==GetCanonicalSourceID(),HasExploredHerbs(SourceID),CanMutate()))
        return Fail(NSLOCTEXT("AskirHerbs","CannotExplore","Nimm zuerst Ruriks Auftrag an und untersuche das ausgewiesene Heilkräutervorkommen."));
    auto* Journal=GetJournal();TGuardValue<bool> Guard(bTransactionInProgress,true);
    Journal->ConsumedQuestTriggers.Add(ExploredReceipt);Journal->ConsumedQuestTriggers.Add(SourceReceipt);
    if(!SyncObjectives())
    {
        Journal->ConsumedQuestTriggers.Remove(ExploredReceipt);Journal->ConsumedQuestTriggers.Remove(SourceReceipt);
        return Fail(NSLOCTEXT("AskirHerbs","ExploreFailed","Die Untersuchung konnte nicht gespeichert werden."));
    }
    LastFailureText=FText::GetEmpty();NotifyProgress(FName(TEXT("SQ_003_A")));return true;
}
bool UAskirHerbMissionComponent::ReturnToRurik()
{
    if(!CanMutate())return Fail(NSLOCTEXT("AskirHerbs","ReturnBusy","Der Bericht kann gerade nicht abgegeben werden."));
    auto* Journal=GetJournal();auto* Data=ResolveQuestData();auto* Owner=GetOwner();
    auto* Bag=Owner->FindComponentByClass<UAskirInventoryComponent>();
    auto* Wallet=Owner->FindComponentByClass<UAskirTradeComponent>();
    auto* Progression=Owner->FindComponentByClass<UAskirProgressionComponent>();
    if(!AskirHerbRules::CanTurnIn(ToRules(GetStage()),Journal&&Journal->HasClaimedReward(GetQuestID()),CanMutate(),
        Data&&IsValid(Data->RewardItem),IsValid(Bag),IsValid(Wallet),IsValid(Progression),Wallet?Wallet->Gold:0))
        return Fail(NSLOCTEXT("AskirHerbs","CannotReturn","Die Untersuchung oder gültige Belohnungsdaten fehlen; ein erneuter Abschluss ist ausgeschlossen."));
    TGuardValue<bool> Guard(bTransactionInProgress,true);const auto Previous=Journal->GetObjectivesForQuest(GetQuestID());
    if(!SyncObjectives(true))return Fail(NSLOCTEXT("AskirHerbs","ReturnFailed","Der Bericht konnte nicht übernommen werden."));
    if(!Journal->TurnInQuestWithRewards(Data))
    {
        Journal->RestoreQuestProgress(GetQuestID(),Previous,EAskirQuestState::Active,false,false);
        return Fail(NSLOCTEXT("AskirHerbs","FullInventory","Die Belohnung passt nicht ins Inventar oder die Auszahlung ist gerade gesperrt."));
    }
    LastFailureText=FText::GetEmpty();NotifyProgress();return true;
}
FText UAskirHerbMissionComponent::GetObjectiveText() const
{
    switch(GetStage())
    {
    case EAskirHerbStage::NotStarted:return NSLOCTEXT("AskirHerbs","Start","Sprich mit Rurik über Heilkräuter für Mara.");
    case EAskirHerbStage::ExploreMeadow:return NSLOCTEXT("AskirHerbs","Meadow","Untersuche das Heilkräutervorkommen auf der Wiese im Dornfelder Umland.");
    case EAskirHerbStage::ReturnToRurik:return NSLOCTEXT("AskirHerbs","Rurik","Berichte Rurik in Dornfeld vom Heilkräutervorkommen.");
    default:return NSLOCTEXT("AskirHerbs","Done","Rurik kennt nun das gesunde Heilkräutervorkommen für Mara.");
    }
}
