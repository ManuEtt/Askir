#include "Frontend/AskirFrontendSubsystem.h"
#include "Frontend/AskirFrontendRules.h"
#include "Game/AskirGameMode.h"
#include "Game/AskirPlayerController.h"
#include "Characters/AskirCharacter.h"
#include "Travel/AskirWorldTravelSubsystem.h"
#include "Save/AskirSaveGame.h"
#include "Save/AskirSaveSlotStore.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/PackageName.h"
#include "UObject/StrongObjectPtr.h"
#include "Subsystems/SubsystemCollection.h"
#include "Containers/Ticker.h"
#include "HAL/PlatformTime.h"

void UAskirFrontendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Collection.InitializeDependency<UAskirWorldTravelSubsystem>();
    FreshStartTicker=FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this,&UAskirFrontendSubsystem::TickFreshStartTimeout),.25f);
    if(GEngine)TravelFailureHandle=GEngine->OnTravelFailure().AddUObject(this,&UAskirFrontendSubsystem::HandleTravelFailure);
}
void UAskirFrontendSubsystem::Deinitialize()
{
    if(GEngine&&TravelFailureHandle.IsValid())GEngine->OnTravelFailure().Remove(TravelFailureHandle);
    if(FreshStartTicker.IsValid()){FTSTicker::GetCoreTicker().RemoveTicker(FreshStartTicker);FreshStartTicker.Reset();}
    FreshStartDeadline=0.;
    FreshStartBackup=nullptr;SourceWorld.Reset();bFreshStartPending=false;
    Super::Deinitialize();
}
FString UAskirFrontendSubsystem::GetStartWorldPackage()
{
    FString Package;
    if(GConfig)GConfig->GetString(TEXT("/Script/EngineSettings.GameMapsSettings"),TEXT("GameDefaultMap"),Package,GEngineIni);
    Package=FPackageName::ObjectPathToPackageName(Package);
    return Package;
}
bool UAskirFrontendSubsystem::HasPendingTravel() const
{
    const auto* State=GetGameInstance()?GetGameInstance()->GetSubsystem<UAskirWorldTravelSubsystem>():nullptr;
    return State&&State->HasPendingTravel();
}
bool UAskirFrontendSubsystem::CanContinue() const
{
    if(bFreshStartPending||HasPendingTravel())return false;
    TStrongObjectPtr<UAskirSaveGame> Save(AskirSaveSlots::ReadLatest(TEXT("AskirQuick")));
    if(!Save)return false;
    const FString Package=Save->SavedMapPackageName.IsEmpty()?FString::Printf(TEXT("/Game/Askir/WorldV42/%s"),*Save->SavedMapName.ToString()):Save->SavedMapPackageName;
    return UAskirWorldTravelSubsystem::IsUsableWorldPackage(Package,Save->SavedMapName);
}
bool UAskirFrontendSubsystem::ContinueGame(AAskirGameMode* Mode)
{
    if(!IsValid(Mode)||Mode->IsPersistenceBusy()||!CanContinue())return false;
    if(!Mode->LoadGame(TEXT("AskirQuick")))
    {LastError=NSLOCTEXT("AskirUI","ContinueFailed","Der Spielstand konnte nicht geladen werden.");return false;}
    MarkSessionActive();LastError=FText::GetEmpty();return true;
}
bool UAskirFrontendSubsystem::CanStartNewGame(AAskirGameMode* Mode) const
{
    AskirFrontendRules::StartReadiness State;
    State.ModeValid=IsValid(Mode);
    State.RuntimeReady=State.ModeValid&&Mode->IsRuntimeReadyForFrontend();
    State.PersistenceBusy=State.ModeValid&&Mode->IsPersistenceBusy();
    State.FreshStartPending=bFreshStartPending;State.AuthoritativeTravelPending=HasPendingTravel();
    const FString Package=GetStartWorldPackage();
    State.TargetWorldPackageValid=UAskirWorldTravelSubsystem::IsUsableWorldPackage(Package,FName(*FPackageName::GetShortName(Package)));
    return AskirFrontendRules::CanPrepareStart(State);
}
bool UAskirFrontendSubsystem::StartNewGame(AAskirGameMode* Mode,bool bDiscardConfirmed)
{
    if(!AskirFrontendRules::CanStartSession(CanStartNewGame(Mode),bHasActiveSession,bDiscardConfirmed))return false;
    const FString Package=GetStartWorldPackage();
    const FName Current(*UGameplayStatics::GetCurrentLevelName(Mode,true));
    // The untouched startup world is already a fresh game. Existing saves are not touched.
    if(!bHasActiveSession&&Current==FName(*FPackageName::GetShortName(Package)))
    {MarkSessionActive();LastError=FText::GetEmpty();return true;}
    double Deadline=0.;
    if(!AskirFrontendRules::MakeFreshStartDeadline(FPlatformTime::Seconds(),Deadline))return false;
    UAskirSaveGame* Backup=Mode->CaptureSnapshot();
    if(!Backup)return false;
    // Keep the complete outgoing session authoritative until a distinct world is ready.
    FreshStartDeadline=Deadline;
    FreshStartBackup=Backup;SourceWorld=Mode->GetWorld();FreshStartPackage=Package;bFreshStartPending=true;
    LastError=FText::GetEmpty();
    if(auto* Player=Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(Mode,0)))Player->SuspendForWorldTravel();
    if(auto* PC=Cast<AAskirPlayerController>(UGameplayStatics::GetPlayerController(Mode,0)))PC->SetMenuInput(true,false);
    UGameplayStatics::OpenLevel(Mode,FName(*Package));
    return true;
}
bool UAskirFrontendSubsystem::CommitFreshStart(AAskirGameMode* Mode)
{
    UWorld* World=IsValid(Mode)?Mode->GetWorld():nullptr;
    auto* Player=World?Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(Mode,0)):nullptr;
    AskirFrontendRules::FreshWorldReadiness State;
    State.FreshStartPending=bFreshStartPending;State.HasSourceBackup=IsValid(FreshStartBackup.Get());
    State.ModeValid=IsValid(Mode);State.RuntimeReady=State.ModeValid&&Mode->IsRuntimeReadyForFrontend();
    State.AuthoritativeTravelPending=HasPendingTravel();State.WorldValid=IsValid(World);
    State.IsOriginalSourceWorld=World&&World==SourceWorld.Get();State.WorldTearingDown=World&&World->bIsTearingDown;
    State.TargetMapMatches=World&&FName(*UGameplayStatics::GetCurrentLevelName(Mode,true))==FName(*FPackageName::GetShortName(FreshStartPackage));
    State.ControlledPlayerReady=IsValid(Player)&&Player->IsPlayerControlled();
    if(!AskirFrontendRules::CanCommitFreshWorld(State))return false;
    if(auto* State=GetGameInstance()->GetSubsystem<UAskirWorldTravelSubsystem>())
    {
        // No fallible operation remains; only this confirmed fresh world replaces the ledger.
        State->LootByMap.Reset();State->MerchantsByMap.Reset();State->ArrivalGraceUntilSeconds=World->GetTimeSeconds()+2.0;
    }
    FreshStartDeadline=0.;
    bFreshStartPending=false;bHasActiveSession=true;FreshStartBackup=nullptr;SourceWorld.Reset();FreshStartPackage.Reset();
    Player->RestoreAfterLoad();
    LastError=FText::GetEmpty();return true;
}
void UAskirFrontendSubsystem::HandleTravelFailure(UWorld* World,ETravelFailure::Type FailureType,const FString& Error)
{
    if(!bFreshStartPending)return;
    if(World&&World->GetGameInstance()!=GetGameInstance())return;
    LastError=NSLOCTEXT("AskirUI","NewGameFailed","Das neue Spiel konnte nicht gestartet werden. Dein bisheriger Fortschritt bleibt erhalten.");
    UE_LOG(LogTemp,Warning,TEXT("Askir fresh start failed (%d): %s"),static_cast<int32>(FailureType),*Error);
    RecoverFreshStartFailure();
}
bool UAskirFrontendSubsystem::TickFreshStartTimeout(float DeltaSeconds)
{
    if(bFreshStartPending&&AskirFrontendRules::FreshStartExpired(FPlatformTime::Seconds(),FreshStartDeadline))
    {
        LastError=NSLOCTEXT("AskirUI","NewGameTimedOut","Das neue Spiel konnte nicht rechtzeitig geladen werden. Dein bisheriger Fortschritt bleibt erhalten.");
        UE_LOG(LogTemp,Warning,TEXT("Askir fresh start timed out before target readiness"));
        RecoverFreshStartFailure();
    }
    return true;
}
void UAskirFrontendSubsystem::RecoverFreshStartFailure()
{
    if(!bFreshStartPending)return;
    UWorld* Active=GetGameInstance()?GetGameInstance()->GetWorld():nullptr;
    auto* State=GetGameInstance()?GetGameInstance()->GetSubsystem<UAskirWorldTravelSubsystem>():nullptr;
    auto* Player=Active?Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(Active,0)):nullptr;
    if(AskirFrontendRules::CanResumeLiveSource(IsValid(Active),Active&&Active==SourceWorld.Get(),
        Active&&Active->bIsTearingDown,IsValid(Player)&&IsValid(Player->GetController())))
    {
        // The old live actors and GI ledgers were never reset.
        FreshStartDeadline=0.;bFreshStartPending=false;FreshStartBackup=nullptr;FreshStartPackage.Reset();SourceWorld.Reset();
        Player->RestoreAfterLoad();
        if(auto* PC=Cast<AAskirPlayerController>(UGameplayStatics::GetPlayerController(Active,0)))PC->ShowNotification(LastError);
        return;
    }
    // If Browse destroyed the source, retain a strong snapshot through the normal recovery flow.
    if(State&&FreshStartBackup&&!State->HasPendingTravel()&&State->QueueSnapshot(FreshStartBackup->SavedMapName,NAME_None,FreshStartBackup,true))
    {
        State->MarkSourceStateUnavailable();State->MarkArrivalFailed(LastError);FreshStartDeadline=0.;bFreshStartPending=false;FreshStartBackup=nullptr;FreshStartPackage.Reset();SourceWorld.Reset();
        bHasActiveSession=true;
        auto* Controller=Active?Cast<AAskirPlayerController>(UGameplayStatics::GetPlayerController(Active,0)):nullptr;
        if(AskirFrontendRules::NeedsAutomaticRecovery(IsValid(Active)&&!Active->bIsTearingDown,IsValid(Controller)&&Controller->IsLocalController(),
            IsValid(Controller)&&Controller->GetLocalPlayer()!=nullptr,IsValid(Controller)&&Controller->HasRuntimeHUD()))
        {
            // Without a recovery-menu owner there is no button to press. Hand the
            // same retained snapshot back once; the normal arrival subsystem owns
            // all later failures, so this cannot create an automatic retry loop.
            FString ReturnPackage;
            if(State->RedirectToSavedWorld(ReturnPackage))
                UGameplayStatics::OpenLevel(GetGameInstance(),FName(*ReturnPackage));
            else UE_LOG(LogTemp,Error,TEXT("Askir fresh start recovery blocked: captured source world is unavailable; snapshot retained"));
        }
    }
    // Existing authoritative handoffs win; a still-retained backup is never silently cleared.
}
