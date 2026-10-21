#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirDeliveryMissionComponent.generated.h"

class UAskirQuestComponent;
class UAskirQuestData;

UENUM(BlueprintType)
enum class EAskirDeliveryStage : uint8 { NotStarted, DefeatBandits, RecoverCargo, ReturnToTorben, Completed };

/** SQ_001 uses the existing quest ledger, dialogue flags and faction save fields. */
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirDeliveryMissionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UAskirDeliveryMissionComponent();
    /** Optional authored/test data. Canonical SQ_001 rewards and objectives are validated. */
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Delivery") TObjectPtr<UAskirQuestData> QuestData;
    UFUNCTION(BlueprintCallable,Category="Askir|Delivery") bool AcceptDelivery(FName ChoiceID);
    /** Native relay boundary: caller must validate real player kill attribution. */
    bool CreditBandit(FName UniqueBanditID);
    UFUNCTION(BlueprintCallable,Category="Askir|Delivery") bool RecoverCargo(FName CargoID);
    UFUNCTION(BlueprintCallable,Category="Askir|Delivery") bool ReturnToTorben();
    UFUNCTION(BlueprintPure,Category="Askir|Delivery") EAskirDeliveryStage GetStage() const;
    UFUNCTION(BlueprintPure,Category="Askir|Delivery") int32 GetBanditCount() const;
    UFUNCTION(BlueprintPure,Category="Askir|Delivery") FName GetAcceptedChoice() const;
    UFUNCTION(BlueprintPure,Category="Askir|Delivery") bool HasCreditedBandit(FName UniqueBanditID) const;
    UFUNCTION(BlueprintPure,Category="Askir|Delivery") bool HasRecoveredCargo(FName CargoID) const;
    UFUNCTION(BlueprintPure,Category="Askir|Delivery") FText GetObjectiveText() const;
    UFUNCTION(BlueprintPure,Category="Askir|Delivery") FText GetLastFailureText() const {return LastFailureText;}
    /** Lazily reconstruct canonical reward data after travel/load; grants no rewards or quest receipts. */
    UAskirQuestData* ResolveQuestData();
    /** Repairs objective mirrors from the persistent receipts; does not grant rewards. */
    UFUNCTION(BlueprintCallable,Category="Askir|Delivery|Save") void RefreshFromQuestLedger();
    bool IsTransactionInProgress() const {return bTransactionInProgress;}
    static FName GetQuestID() {return FName(TEXT("SQ_001"));}
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UFUNCTION() void OnQuestProgressChanged(FName QuestID,FName ObjectiveID);
    UAskirQuestComponent* GetJournal() const;
    bool CanMutate() const;
    bool HasCargoReceipt() const;
    bool SyncObjectives(bool bReturned=false);
    bool Fail(const FText& Reason);
    bool bTransactionInProgress=false;
    UPROPERTY(Transient) FText LastFailureText;
};
