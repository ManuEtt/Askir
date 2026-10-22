#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirHerbMissionComponent.generated.h"

class UAskirQuestComponent;
class UAskirQuestData;

UENUM(BlueprintType)
enum class EAskirHerbStage : uint8 { NotStarted, ExploreMeadow, ReturnToRurik, Completed };

/** SQ_003 authoritative state is the existing saved quest/choice ledger. */
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirHerbMissionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UAskirHerbMissionComponent();
    /** Optional authored/test data, validated against canonical objectives/rewards. */
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Herbs") TObjectPtr<UAskirQuestData> QuestData;
    UFUNCTION(BlueprintCallable,Category="Askir|Herbs") bool AcceptHerbs(FName ChoiceID);
    /** Trusted native event boundary: the real patch validates position and sight. */
    UFUNCTION(BlueprintCallable,Category="Askir|Herbs") bool ExploreHerbs(FName SourceID);
    UFUNCTION(BlueprintCallable,Category="Askir|Herbs") bool ReturnToRurik();
    UFUNCTION(BlueprintPure,Category="Askir|Herbs") EAskirHerbStage GetStage() const;
    UFUNCTION(BlueprintPure,Category="Askir|Herbs") FName GetAcceptedChoice() const;
    UFUNCTION(BlueprintPure,Category="Askir|Herbs") bool HasExploredHerbs(FName SourceID) const;
    UFUNCTION(BlueprintPure,Category="Askir|Herbs") FText GetObjectiveText() const;
    UFUNCTION(BlueprintPure,Category="Askir|Herbs") FText GetLastFailureText() const {return LastFailureText;}
    UFUNCTION(BlueprintCallable,Category="Askir|Herbs|Save") void RefreshFromQuestLedger();
    /** Lazily resolves a real reward after travel/load; creates no inventory item. */
    UAskirQuestData* ResolveQuestData();
    bool IsTransactionInProgress() const {return bTransactionInProgress;}
    static FName GetQuestID() {return FName(TEXT("SQ_003"));}
    static FName GetCanonicalSourceID() {return FName(TEXT("SQ003_Meadow_V84"));}
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UFUNCTION() void OnQuestProgressChanged(FName QuestID,FName ObjectiveID);
    UAskirQuestComponent* GetJournal() const;
    bool CanMutate() const;
    bool HasExplorationReceipt() const;
    bool SyncObjectives(bool bReturned=false);
    void NotifyProgress(FName ObjectiveID=NAME_None);
    bool Fail(const FText& Reason);
    bool bTransactionInProgress=false;
    UPROPERTY(Transient) FText LastFailureText;
};
