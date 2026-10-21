#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AskirConversationProbe.generated.h"

class AAskirCharacter;
class AActor;
class AAskirGameMode;
class UAskirConversationComponent;
class UAskirSaveGame;

/** Reflected callback receiver for actual conversation/domain re-entry tests. */
UCLASS()
class UAskirConversationProbe : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() TObjectPtr<UAskirConversationComponent> Conversation;
    UPROPERTY() TObjectPtr<AAskirCharacter> Player;
    UPROPERTY() TObjectPtr<AActor> Speaker;
    UPROPERTY() TObjectPtr<AAskirGameMode> Mode;
    UPROPERTY() TObjectPtr<UAskirSaveGame> Snapshot;
    bool bRequestClose=false;
    bool bDestroySpeaker=false;
    bool bDestroyPlayer=false;
    bool bObserved=false;
    bool bObservedTransaction=false;
    bool bObservedDomainReceipt=false;
    bool bReentrantChoiceAccepted=false;
    bool bReentrantBeginAccepted=false;
    bool bCaptureBlocked=false;
    bool bRestoreBlocked=false;
    UFUNCTION() void ObserveReputationChange(FName Faction,int32 NewReputation);
};
