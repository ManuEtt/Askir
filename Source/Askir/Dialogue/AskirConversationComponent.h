#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialogue/AskirConversationTypes.h"
#include "AskirConversationComponent.generated.h"

class AAskirCharacter;
class UAskirDialogueComponent;

/** Local dialogue cursor; all durable flags remain in the existing player save ledger. */
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirConversationComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UAskirConversationComponent();
    virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UFUNCTION(BlueprintCallable,Category="Askir|Dialogue") bool BeginConversation(AActor* Speaker,FName DialogueID=NAME_None);
    UFUNCTION(BlueprintCallable,Category="Askir|Dialogue") bool SelectChoice(FName ChoiceID,int64 ExpectedRevision);
    UFUNCTION(BlueprintCallable,Category="Askir|Dialogue") void EndConversation();
    UFUNCTION(BlueprintPure,Category="Askir|Dialogue") bool IsActive() const {return bActive;}
    UFUNCTION(BlueprintPure,Category="Askir|Dialogue") int64 GetRevision() const {return Revision;}
    UFUNCTION(BlueprintPure,Category="Askir|Dialogue") FAskirConversationNode GetCurrentNode() const;
    UFUNCTION(BlueprintPure,Category="Askir|Dialogue") TArray<FAskirConversationChoice> GetAvailableChoices() const;
    UFUNCTION(BlueprintPure,Category="Askir|Dialogue") AActor* GetSpeaker() const {return SpeakerActor.Get();}
    UFUNCTION(BlueprintPure,Category="Askir|Dialogue") bool CanContinueConversation() const;
    UFUNCTION(BlueprintPure,Category="Askir|Dialogue") FText GetLastFailureText() const {return LastFailureText;}
    bool IsTransactionInProgress() const {return bTransactionInProgress;}
    /** Exposed for native editor validation; does not install or mutate a session. */
    static bool ValidateGraph(const FAskirConversationGraph& Graph);
    UFUNCTION(BlueprintPure,Category="Askir|Dialogue") static bool IsKnownDialogue(FName DialogueID);
private:
    UPROPERTY(Transient) FAskirConversationGraph ActiveGraph;
    UPROPERTY(Transient) TWeakObjectPtr<AActor> SpeakerActor;
    UPROPERTY(Transient) FText LastFailureText;
    FName CurrentNode=NAME_None;
    int64 Revision=0;
    bool bActive=false;
    bool bTransactionInProgress=false;
    bool bEndRequested=false;
    bool Fail(const FText& Text);
    void ClearCursor();
    bool IsContextValid(AActor* Speaker) const;
    bool ConditionsMatch(const TArray<FName>& Required,const TArray<FName>& Forbidden) const;
    bool IsActionReady(const FAskirConversationChoice& Choice) const;
    bool ExecuteAction(const FAskirConversationChoice& Choice);
    bool BuildLegacyGraph(AActor* Speaker,FAskirConversationGraph& Out) const;
    const FAskirConversationNode* FindNode(FName ID) const;
    AAskirCharacter* GetPlayer() const;
    UAskirDialogueComponent* GetFlagLedger() const;
};
