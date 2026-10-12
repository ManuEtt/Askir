#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AskirGameMode.generated.h"
class AAskirDemoInteractable;
class AAskirNPC;
class UAskirSaveGame;
UCLASS()
class ASKIR_API AAskirGameMode:public AGameModeBase
{
    GENERATED_BODY()
public:
    AAskirGameMode();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UFUNCTION(BlueprintCallable) bool SaveGame(const FString& SlotName);
    UFUNCTION(BlueprintCallable) bool LoadGame(const FString& SlotName, bool bRestorePosition=true);
    /** Capture/restore are shared by disk saves and in-memory OpenLevel handoff. */
    UAskirSaveGame* CaptureSnapshot();
    bool RestoreSnapshot(UAskirSaveGame* Snapshot, bool bRestorePosition=true);
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Demo") bool bEnableLegacyDemo=false;
    UFUNCTION(BlueprintCallable) void DemoInteract();
    UFUNCTION(BlueprintCallable) void DemoQuickSave();
    UFUNCTION(BlueprintCallable) void DemoQuickLoad();
    UFUNCTION(BlueprintPure) FString GetDemoObjective() const;
    UPROPERTY(BlueprintReadOnly) int32 DemoStage=0;
    UPROPERTY(BlueprintReadOnly) int32 DemoSupplies=0;
    UPROPERTY(BlueprintReadOnly) bool bDemoFinished=false;
private:
    void RestorePendingArrival();
    void LogRuntimeReadiness();
    void BuildDemoScene();
    void MakeBlock(FVector Position,FVector Size);
    AAskirDemoInteractable* MakeInteractable(FVector Position,int32 Kind,const TCHAR* Label);
    UPROPERTY() TObjectPtr<AAskirNPC> DemoEnemy;
    UPROPERTY() TArray<TObjectPtr<AAskirDemoInteractable>> DemoObjects;
    float HudAccumulator=0.f;
    bool bDemoSceneReady=false;
    bool bPersistenceOperation=false;
    int32 ArrivalRestoreAttempts=0;

};
