#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirDornfeldMissionComponent.generated.h"

/** Persistent progression for the Dornfeld vertical slice. Editor events supply stable IDs. */
UENUM(BlueprintType)
enum class EAskirDornfeldStage : uint8 { NotStarted, CollectShards, DefeatGuardians, OpenChest, ReturnToElder, Completed };
USTRUCT(BlueprintType)
struct FAskirDornfeldMissionSave
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) EAskirDornfeldStage Stage=EAskirDornfeldStage::NotStarted;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) TArray<FName> ShardIDs;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) TArray<FName> GuardianIDs;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) FName ChestID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) bool bRewardClaimed=false;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAskirDornfeldStageChanged,EAskirDornfeldStage,Stage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAskirDornfeldObjectiveChanged,FText,Objective,EAskirDornfeldStage,Stage);
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirDornfeldMissionComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UAskirDornfeldMissionComponent();
 UPROPERTY(BlueprintAssignable,Category="Askir|Dornfeld") FAskirDornfeldStageChanged OnStageChanged;
 UPROPERTY(BlueprintAssignable,Category="Askir|Dornfeld") FAskirDornfeldObjectiveChanged OnObjectiveChanged;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Dornfeld") FAskirDornfeldMissionSave State;
 UFUNCTION(BlueprintCallable,Category="Askir|Dornfeld") bool TalkToElder();
 UFUNCTION(BlueprintCallable,Category="Askir|Dornfeld") bool CollectShard(FName UniqueShardID,bool bNotify=true);
 UFUNCTION(BlueprintCallable,Category="Askir|Dornfeld") bool DefeatGuardian(FName UniqueGuardianID);
 UFUNCTION(BlueprintCallable,Category="Askir|Dornfeld") bool OpenMissionChest(FName UniqueChestID,bool bNotify=true);
 UFUNCTION(BlueprintPure,Category="Askir|Dornfeld") bool CanCollectShard(FName UniqueShardID) const;
 UFUNCTION(BlueprintPure,Category="Askir|Dornfeld") bool CanDefeatGuardian(FName UniqueGuardianID) const;
 UFUNCTION(BlueprintPure,Category="Askir|Dornfeld") bool CanOpenMissionChest(FName UniqueChestID) const;
 UFUNCTION(BlueprintPure,Category="Askir|Dornfeld") FText GetObjectiveText() const;
 /** Call after a late-created HUD subscribes to OnObjectiveChanged. */
 UFUNCTION(BlueprintCallable,Category="Askir|Dornfeld") void RefreshObjective() { BroadcastObjective(); }
 UFUNCTION(BlueprintPure,Category="Askir|Dornfeld") EAskirDornfeldStage GetStage() const {return State.Stage;}
 UFUNCTION(BlueprintPure,Category="Askir|Dornfeld") int32 GetShardCount() const {return State.ShardIDs.Num();}
 UFUNCTION(BlueprintPure,Category="Askir|Dornfeld") int32 GetGuardianCount() const {return State.GuardianIDs.Num();}
 UFUNCTION(BlueprintPure,Category="Askir|Dornfeld") FAskirDornfeldMissionSave CaptureMission() const {return State;}
 UFUNCTION(BlueprintCallable,Category="Askir|Dornfeld") void RestoreMission(const FAskirDornfeldMissionSave& Saved,bool bNotify=true);
 /** Notify listeners once the entire save snapshot has been installed. */
 UFUNCTION(BlueprintCallable,Category="Askir|Dornfeld") void NotifyMissionRestored();
private:
 void SetStage(EAskirDornfeldStage Next);
 void BroadcastObjective();
 void SyncQuestJournal(bool bNotify=true);
 bool bClaimingReward=false;
};
