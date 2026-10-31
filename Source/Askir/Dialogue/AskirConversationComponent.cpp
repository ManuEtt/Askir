#include "Dialogue/AskirConversationComponent.h"
#include "Dialogue/AskirConversationRules.h"
#include "Dialogue/AskirHerbConversationRules.h"
#include "Dialogue/AskirConversationCatalog.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Combat/AskirCombatComponent.h"
#include "NPC/AskirDialogueComponent.h"
#include "NPC/AskirNPCInteractionComponent.h"
#include "Mission/AskirDeliveryQuestActor.h"
#include "Mission/AskirDeliveryMissionComponent.h"
#include "Mission/AskirHerbMissionComponent.h"
#include "Game/AskirGameMode.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Social/AskirReputationComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

namespace
{
std::string Key(FName ID)
{
    return ID.IsNone()?std::string():std::string(TCHAR_TO_UTF8(*ID.ToString().ToLower()));
}
std::vector<std::string> Keys(const TArray<FName>& IDs)
{
    std::vector<std::string> Result;Result.reserve(IDs.Num());
    for(FName ID:IDs)Result.push_back(Key(ID));return Result;
}
AskirConversationRules::Graph Portable(const FAskirConversationGraph& Source)
{
    AskirConversationRules::Graph Result;Result.ID=Key(Source.ID);Result.StartNode=Key(Source.StartNode);
    for(const auto& Node:Source.Nodes)
    {
        AskirConversationRules::Node NativeNode;NativeNode.ID=Key(Node.NodeID);
        for(const auto& Choice:Node.Choices)
            NativeNode.Choices.push_back({Key(Choice.ChoiceID),Key(Choice.NextNode),Keys(Choice.RequiredFlags),Keys(Choice.ForbiddenFlags)});
        Result.Nodes.push_back(std::move(NativeNode));
    }
    for(const auto& Entry:Source.Entries)
        Result.Entries.push_back({Entry.Condition.DeliveryStage,Keys(Entry.Condition.RequiredFlags),Keys(Entry.Condition.ForbiddenFlags),Key(Entry.NodeID)});
    return Result;
}
int DeliveryStage(const AActor* Player)
{
    const auto* Delivery=Player?Player->FindComponentByClass<UAskirDeliveryMissionComponent>():nullptr;
    return Delivery?static_cast<int>(Delivery->GetStage()):0;
}
bool Advance(int64& Revision)
{
    std::int64_t Next=0;if(!AskirConversationRules::NextRevision(static_cast<std::int64_t>(Revision),Next))return false;
    Revision=static_cast<int64>(Next);return true;
}
}

UAskirConversationComponent::UAskirConversationComponent()
{
    PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.bStartWithTickEnabled=false;
    PrimaryComponentTick.TickInterval=.1f;
}
AAskirCharacter* UAskirConversationComponent::GetPlayer() const
{
    auto* Player=Cast<AAskirCharacter>(GetOwner());return IsValid(Player)?Player:nullptr;
}
UAskirDialogueComponent* UAskirConversationComponent::GetFlagLedger() const
{
    const AAskirCharacter* Player=GetPlayer();return Player?Player->FindComponentByClass<UAskirDialogueComponent>():nullptr;
}
bool UAskirConversationComponent::Fail(const FText& Text){LastFailureText=Text;return false;}
bool UAskirConversationComponent::IsContextValid(AActor* Speaker) const
{
    const AAskirCharacter* Player=GetPlayer();
    if(!Player||Player->IsActorBeingDestroyed()||!Player->IsPlayerControlled()||Player->IsDead()||Player->IsWorldTravelPending()||!Player->CombatComponent||
        !Player->CombatComponent->IsDamageEnabled()||!GetWorld()||GetWorld()->bIsTearingDown||UGameplayStatics::IsGamePaused(this)||!GetFlagLedger()||
        !IsValid(Speaker)||Speaker->IsActorBeingDestroyed()||Speaker==Player||Speaker->GetWorld()!=GetWorld()||Speaker->IsHidden()||
        Player->GetActorLocation().ContainsNaN()||Speaker->GetActorLocation().ContainsNaN())return false;
    if(const auto* Mode=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))if(Mode->IsPersistenceBusy())return false;
    if(const auto* Combat=Speaker->FindComponentByClass<UAskirCombatComponent>())
        if(Combat->IsDead()||!Combat->IsDamageEnabled())return false;
    if(const auto* Giver=Cast<AAskirDeliveryQuestActor>(Speaker))
    {
        if(Giver->Role!=EAskirDeliveryQuestRole::Giver||!Giver->CanInteractWith(const_cast<AAskirCharacter*>(Player)))return false;
    }
    else
    {
        if(const auto* NPC=Cast<AAskirNPC>(Speaker))if(NPC->bHostile||!NPC->IsEncounterActive())return false;
        const auto* Interaction=Speaker->FindComponentByClass<UAskirNPCInteractionComponent>();
        if(!Interaction||!FMath::IsFinite(Interaction->MaxInteractionDistance)||Interaction->MaxInteractionDistance<=0.f||
            !Interaction->CanInteract(const_cast<AAskirCharacter*>(Player)))return false;
    }
    FVector Eye;FRotator Rotation;Player->GetActorEyesViewPoint(Eye,Rotation);
    const FVector Point=Speaker->GetActorLocation()+FVector(0.,0.,40.);
    if(Eye.ContainsNaN()||Point.ContainsNaN())return false;
    FHitResult Hit;FCollisionQueryParams Query(SCENE_QUERY_STAT(AskirConversationSight),false,Player);
    return !GetWorld()->LineTraceSingleByChannel(Hit,Eye,Point,ECC_Visibility,Query)||Hit.GetActor()==Speaker;
}
bool UAskirConversationComponent::CanContinueConversation() const
{
    return bActive&&!bTransactionInProgress&&IsContextValid(SpeakerActor.Get())&&FindNode(CurrentNode);
}
bool UAskirConversationComponent::ValidateGraph(const FAskirConversationGraph& Graph)
{
    // Bound before allocating a portable representation of editor-authored data.
    if(Graph.Nodes.Num()<1||Graph.Nodes.Num()>static_cast<int32>(AskirConversationRules::MaxNodes)||
        Graph.Entries.Num()>static_cast<int32>(AskirConversationRules::MaxEntries))return false;
    for(const auto& Node:Graph.Nodes)
    {
        if(Node.Choices.Num()>static_cast<int32>(AskirConversationRules::MaxChoicesPerNode)||Node.Text.ToString().Len()>4096||
            Node.Speaker.ToString().Len()>256)return false;
        for(const auto& Choice:Node.Choices)
        {
            if(Choice.Label.IsEmpty()||Choice.Label.ToString().Len()>256||Choice.RequiredFlags.Num()>16||Choice.ForbiddenFlags.Num()>16||
                Choice.GrantedFlag.ToString().StartsWith(TEXT("SQ_"),ESearchCase::IgnoreCase))return false;
        }
    }
    for(const auto& Entry:Graph.Entries)if(Entry.Condition.RequiredFlags.Num()>16||Entry.Condition.ForbiddenFlags.Num()>16)return false;
    return AskirConversationRules::ValidateGraph(Portable(Graph));
}
bool UAskirConversationComponent::IsKnownDialogue(FName DialogueID)
{
    FAskirConversationGraph Graph;
    return !DialogueID.IsNone()&&AskirConversationCatalog::Build(DialogueID,Graph)&&Graph.ID==DialogueID&&ValidateGraph(Graph);
}
bool UAskirConversationComponent::BuildLegacyGraph(AActor* Speaker,FAskirConversationGraph& Out) const
{
    const auto* Source=IsValid(Speaker)?Speaker->FindComponentByClass<UAskirDialogueComponent>():nullptr;
    if(!Source||Source->Nodes.Num()<1||Source->Nodes.Num()>static_cast<int32>(AskirConversationRules::MaxNodes))return false;
    FAskirConversationGraph Graph;Graph.ID=FName(TEXT("LegacyConversation"));Graph.StartNode=Source->StartNode;
    for(const auto& Node:Source->Nodes)
    {
        if(Node.Choices.Num()>static_cast<int32>(AskirConversationRules::MaxChoicesPerNode))return false;
        FAskirConversationNode Converted;Converted.NodeID=Node.NodeID;Converted.Speaker=Node.Speaker;Converted.Text=Node.Text;
        for(int32 Index=0;Index<Node.Choices.Num();++Index)
        {
            const auto& Choice=Node.Choices[Index];FAskirConversationChoice ConvertedChoice;
            ConvertedChoice.ChoiceID=FName(*FString::Printf(TEXT("LegacyChoice_%d"),Index));ConvertedChoice.Label=Choice.Label;
            ConvertedChoice.NextNode=Choice.NextNode;ConvertedChoice.GrantedFlag=Choice.GrantedFlag;
            if(!Choice.RequiredFlag.IsNone())ConvertedChoice.RequiredFlags.Add(Choice.RequiredFlag);
            Converted.Choices.Add(ConvertedChoice);
        }
        Graph.Nodes.Add(Converted);
    }
    if(!ValidateGraph(Graph))return false;Out=MoveTemp(Graph);return true;
}
bool UAskirConversationComponent::BeginConversation(AActor* Speaker,FName DialogueID)
{
    AAskirCharacter* Player=GetPlayer();
    if(bActive||bTransactionInProgress||!Player||!Player->CanUseGameplayInput()||!IsContextValid(Speaker))
        return Fail(NSLOCTEXT("AskirConversation","BeginUnavailable","Dieses Gespräch kann gerade nicht begonnen werden."));
    if(DialogueID.IsNone())if(const auto* NPC=Cast<AAskirNPC>(Speaker))DialogueID=NPC->ConversationID;
    FAskirConversationGraph Graph;
    const bool bBuilt=DialogueID.IsNone()?BuildLegacyGraph(Speaker,Graph):AskirConversationCatalog::Build(DialogueID,Graph);
    if(!bBuilt||!ValidateGraph(Graph)||(!DialogueID.IsNone()&&Graph.ID!=DialogueID))
        return Fail(NSLOCTEXT("AskirConversation","InvalidGraph","Die Gesprächsdaten sind nicht verfügbar oder ungültig."));
    const std::string Entry=AskirConversationRules::ChooseEntry(Portable(Graph),DeliveryStage(Player),Keys(GetEffectiveConversationFlags()));
    FName Start=NAME_None;
    for(const auto& Node:Graph.Nodes)if(Key(Node.NodeID)==Entry){Start=Node.NodeID;break;}
    if(Start.IsNone()||!Advance(Revision))
        return Fail(NSLOCTEXT("AskirConversation","RevisionExhausted","Das Gespräch kann nicht sicher fortgesetzt werden."));
    ActiveGraph=MoveTemp(Graph);SpeakerActor=Speaker;CurrentNode=Start;bActive=true;bEndRequested=false;
    LastFailureText=FText::GetEmpty();SetComponentTickEnabled(true);return true;
}
const FAskirConversationNode* UAskirConversationComponent::FindNode(FName ID) const
{
    for(const auto& Node:ActiveGraph.Nodes)if(Node.NodeID==ID)return &Node;return nullptr;
}
FAskirConversationNode UAskirConversationComponent::GetCurrentNode() const
{
    const auto* Node=bActive?FindNode(CurrentNode):nullptr;return Node?*Node:FAskirConversationNode();
}
TArray<FName> UAskirConversationComponent::GetEffectiveConversationFlags() const
{
    const auto* Ledger=GetFlagLedger();const auto* Player=GetPlayer();
    const auto* Herbs=Player?Player->FindComponentByClass<UAskirHerbMissionComponent>():nullptr;
    const auto Effective=AskirHerbConversationRules::EffectiveFlags(Ledger?Keys(Ledger->GetFlags()):std::vector<std::string>(),
        Herbs?static_cast<int>(Herbs->GetStage()):-1);
    TArray<FName> Result;Result.Reserve(static_cast<int32>(Effective.size()));
    for(const auto& Flag:Effective)Result.Add(FName(UTF8_TO_TCHAR(Flag.c_str())));
    return Result; // Virtual SQ_003_STAGE_* flags are never written to the save ledger.
}
bool UAskirConversationComponent::ConditionsMatch(const TArray<FName>& Required,const TArray<FName>& Forbidden) const
{
    return GetFlagLedger()&&AskirConversationRules::ConditionsMatch(Keys(Required),Keys(Forbidden),Keys(GetEffectiveConversationFlags()));
}
bool UAskirConversationComponent::IsActionReady(const FAskirConversationChoice& Choice) const
{
    const AAskirCharacter* Player=GetPlayer();
    auto* Delivery=Player?Player->FindComponentByClass<UAskirDeliveryMissionComponent>():nullptr;
    const auto* Giver=Cast<AAskirDeliveryQuestActor>(SpeakerActor.Get());
    const auto* Journal=Player?Player->FindComponentByClass<UAskirQuestComponent>():nullptr;
    const auto* Bag=Player?Player->FindComponentByClass<UAskirInventoryComponent>():nullptr;
    const bool bGiver=Giver&&Giver->Role==EAskirDeliveryQuestRole::Giver&&Giver->CanInteractWith(const_cast<AAskirCharacter*>(Player));
    const bool bDomainReady=Delivery&&!Delivery->IsTransactionInProgress()&&Journal&&!Journal->IsTransactionInProgress()&&Bag&&!Bag->IsTransactionInProgress()&&
        Player->FindComponentByClass<UAskirReputationComponent>()&&Player->FindComponentByClass<UAskirTradeComponent>()&&Player->FindComponentByClass<UAskirProgressionComponent>();
    if(AskirHerbConversationRules::IsHerbAction(Key(Choice.Action),Key(Choice.Payload)))
    {
        auto* Herbs=Player?Player->FindComponentByClass<UAskirHerbMissionComponent>():nullptr;
        const auto* Rurik=Cast<AAskirNPC>(SpeakerActor.Get());
        const bool bRurik=IsValid(Rurik)&&Rurik->ConversationID==FName(TEXT("D_DORNFELD_RURIK"))&&
            Rurik->ActorHasTag(FName(TEXT("ASKIR_V84_RURIK_GIVER")))&&!Rurik->bHostile&&Rurik->IsEncounterActive();
        const bool bHerbDomainReady=Herbs&&!Herbs->IsTransactionInProgress()&&Journal&&!Journal->IsTransactionInProgress()&&Bag&&!Bag->IsTransactionInProgress()&&
            Player->FindComponentByClass<UAskirReputationComponent>()&&Player->FindComponentByClass<UAskirTradeComponent>()&&Player->FindComponentByClass<UAskirProgressionComponent>();
        if(!AskirHerbConversationRules::ActionReady(Key(Choice.Action),Key(Choice.Payload),Herbs?static_cast<int>(Herbs->GetStage()):-1,bRurik,bHerbDomainReady))return false;
        if(Choice.Action==FName(TEXT("return_herbs")))
        {
            const UAskirQuestData* Data=Herbs->ResolveQuestData();
            if(!IsValid(Data)||!IsValid(Data->RewardItem)||Data->RewardItemQuantity<=0)return false;
            FAskirInventoryEntry Reward;Reward.Item=Data->RewardItem;Reward.Quantity=Data->RewardItemQuantity;
            if(!Bag->CanExchangeItems({}, {Reward}))return false;
        }
        return true;
    }
    if(!AskirConversationRules::ActionReady(Key(Choice.Action),Key(Choice.Payload),DeliveryStage(Player),bGiver,bDomainReady))return false;
    if(Choice.Action==FName(TEXT("return_delivery")))
    {
        // QuestData is transient component state. A fresh player restored from
        // a save or another map has only the durable quest ledger, so readiness
        // must use the same lazy canonical resolver as the reward transaction.
        const UAskirQuestData* Data=Delivery->ResolveQuestData();
        if(!IsValid(Data)||!IsValid(Data->RewardItem)||Data->RewardItemQuantity<=0)return false;
        FAskirInventoryEntry Reward;Reward.Item=Data->RewardItem;Reward.Quantity=Data->RewardItemQuantity;
        if(!Bag->CanExchangeItems({}, {Reward}))return false;
    }
    return true;
}
TArray<FAskirConversationChoice> UAskirConversationComponent::GetAvailableChoices() const
{
    TArray<FAskirConversationChoice> Choices;
    if(!CanContinueConversation())return Choices;
    const auto* Node=FindNode(CurrentNode);if(!Node)return Choices;
    for(const auto& Choice:Node->Choices)if(ConditionsMatch(Choice.RequiredFlags,Choice.ForbiddenFlags))
    {
        FAskirConversationChoice Result=Choice;Result.bEnabled=Choice.bEnabled&&IsActionReady(Choice);Choices.Add(Result);
    }
    return Choices;
}
bool UAskirConversationComponent::ExecuteAction(const FAskirConversationChoice& Choice)
{
    if(Choice.Action.IsNone())return true;
    if(AskirHerbConversationRules::IsHerbAction(Key(Choice.Action),Key(Choice.Payload)))
    {
        auto* Herbs=GetPlayer()?GetPlayer()->FindComponentByClass<UAskirHerbMissionComponent>():nullptr;
        if(!Herbs)return false;
        const bool bResult=Choice.Action==FName(TEXT("return_herbs"))?Herbs->ReturnToRurik():
            Herbs->AcceptHerbs(Choice.Action==FName(TEXT("quest_start"))?FName(TEXT("help")):Choice.Payload);
        if(!bResult&&IsValid(Herbs))LastFailureText=Herbs->GetLastFailureText();return bResult;
    }
    auto* Delivery=GetPlayer()?GetPlayer()->FindComponentByClass<UAskirDeliveryMissionComponent>():nullptr;
    if(!Delivery)return false;
    bool bResult=false;
    if(Choice.Action==FName(TEXT("accept_delivery")))bResult=Delivery->AcceptDelivery(Choice.Payload);
    else if(Choice.Action==FName(TEXT("return_delivery")))bResult=Delivery->ReturnToTorben();
    else if(Choice.Action==FName(TEXT("quest_start"))&&Choice.Payload==FName(TEXT("SQ_001")))bResult=Delivery->AcceptDelivery(FName(TEXT("help")));
    if(!bResult&&IsValid(Delivery))LastFailureText=Delivery->GetLastFailureText();return bResult;
}
bool UAskirConversationComponent::SelectChoice(FName ChoiceID,int64 ExpectedRevision)
{
    if(!AskirConversationRules::CanSelect(bActive,bTransactionInProgress,CanContinueConversation(),Revision,ExpectedRevision))
        return Fail(NSLOCTEXT("AskirConversation","StaleChoice","Diese Gesprächsauswahl ist nicht mehr gültig."));
    const auto* Node=FindNode(CurrentNode);const FAskirConversationChoice* Found=nullptr;
    if(Node)for(const auto& Choice:Node->Choices)if(Choice.ChoiceID==ChoiceID){Found=&Choice;break;}
    if(!Found||!Found->bEnabled||!ConditionsMatch(Found->RequiredFlags,Found->ForbiddenFlags)||!IsActionReady(*Found))
        return Fail(NSLOCTEXT("AskirConversation","ChoiceUnavailable","Diese Antwort ist derzeit nicht verfügbar."));
    const FAskirConversationChoice Choice=*Found;
    // Install the new token and mutation guard before calling quest/faction/reward
    // delegates. Re-entry cannot select/end/restart or capture a partial save.
    if(!Advance(Revision)){ClearCursor();return Fail(NSLOCTEXT("AskirConversation","RevisionExhausted","Das Gespräch kann nicht sicher fortgesetzt werden."));}
    TGuardValue<bool> Guard(bTransactionInProgress,true);
    if(!ExecuteAction(Choice))
    {
        if(LastFailureText.IsEmpty())LastFailureText=NSLOCTEXT("AskirConversation","ActionFailed","Die Auswahl konnte nicht übernommen werden.");
        if(AskirConversationRules::ShouldCloseAfterAction(bEndRequested,IsContextValid(SpeakerActor.Get()),bActive))ClearCursor();
        return false; // Keep the original node and grant no new lore flag.
    }
    // A quest callback may close the HUD, destroy the speaker/player, or start
    // teardown. The domain receipt is already committed; never revive this cursor
    // or dereference a ledger removed by such a callback.
    UAskirDialogueComponent* Ledger=GetFlagLedger();
    if(AskirConversationRules::ShouldCloseAfterAction(bEndRequested,IsContextValid(SpeakerActor.Get()),bActive)||!IsValid(Ledger))
    {
        ClearCursor();LastFailureText=FText::GetEmpty();return true;
    }
    if(!Choice.GrantedFlag.IsNone())Ledger->AddFlag(Choice.GrantedFlag);
    if(Choice.NextNode.IsNone())ClearCursor();else CurrentNode=Choice.NextNode;
    LastFailureText=FText::GetEmpty();return true;
}
void UAskirConversationComponent::ClearCursor()
{
    bActive=false;bEndRequested=false;CurrentNode=NAME_None;SpeakerActor.Reset();ActiveGraph=FAskirConversationGraph();SetComponentTickEnabled(false);
}
void UAskirConversationComponent::EndConversation()
{
    if(bTransactionInProgress){bEndRequested=true;return;}
    if(!bActive)return;
    // Exhaustion permanently prevents another session; the cursor still closes.
    Advance(Revision);ClearCursor();
}
void UAskirConversationComponent::TickComponent(float DeltaTime,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(DeltaTime,Type,Tick);
    if(bActive&&!bTransactionInProgress&&!CanContinueConversation())EndConversation();
}
void UAskirConversationComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    // EndPlay cannot retain external references, including during world teardown.
    Advance(Revision);ClearCursor();Super::EndPlay(Reason);
}
