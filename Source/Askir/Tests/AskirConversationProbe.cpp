#include "Tests/AskirConversationProbe.h"
#include "Dialogue/AskirConversationComponent.h"
#include "Characters/AskirCharacter.h"
#include "Game/AskirGameMode.h"
#include "Quest/AskirQuestComponent.h"
#include "Save/AskirSaveGame.h"

void UAskirConversationProbe::ObserveReputationChange(FName Faction,int32 NewReputation)
{
    bObserved=true;
    if(!Conversation)return;
    bObservedTransaction=Conversation->IsTransactionInProgress();
    const auto* Journal=Player?Player->FindComponentByClass<UAskirQuestComponent>():nullptr;
    bObservedDomainReceipt=Journal&&Journal->HasConsumedQuestTrigger(TEXT("SQ_001_ACCEPTED"));
    bReentrantChoiceAccepted=Conversation->SelectChoice(TEXT("help"),Conversation->GetRevision());
    bReentrantBeginAccepted=Conversation->BeginConversation(Speaker,TEXT("D_SQ_001"));
    bCaptureBlocked=Mode&&Mode->CaptureSnapshot()==nullptr;
    bRestoreBlocked=Mode&&Snapshot&&!Mode->RestoreSnapshot(Snapshot,false);
    if(bRequestClose)Conversation->EndConversation();
    if(bDestroySpeaker&&IsValid(Speaker))Speaker->Destroy();
    if(bDestroyPlayer&&IsValid(Player))Player->Destroy();
}
