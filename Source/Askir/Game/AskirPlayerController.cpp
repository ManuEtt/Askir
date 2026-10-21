#include "Game/AskirPlayerController.h"
#include "UI/AskirHUDWidget.h"
#include "Characters/AskirCharacter.h"
#include "Dialogue/AskirConversationComponent.h"
#include "Frontend/AskirFrontendSubsystem.h"
#include "Travel/AskirWorldTravelSubsystem.h"
#include "Engine/GameInstance.h"
#include "Settings/AskirGameUserSettings.h"

void AAskirPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if(!IsLocalController()||!GetLocalPlayer())return;
    if(auto* Settings=UAskirGameUserSettings::Get())Settings->ApplyNonResolutionSettings();
    SetInputMode(FInputModeGameOnly());
    auto* Frontend=GetGameInstance()?GetGameInstance()->GetSubsystem<UAskirFrontendSubsystem>():nullptr;
    const auto* Travel=GetGameInstance()?GetGameInstance()->GetSubsystem<UAskirWorldTravelSubsystem>():nullptr;
    if(Frontend&&(!Frontend->HasActiveSession()||Frontend->IsFreshStartPending())&&!(Travel&&Travel->HasPendingTravel()))SetMenuInput(true,false);
    RuntimeHUD=CreateWidget<UAskirHUDWidget>(this,UAskirHUDWidget::StaticClass());
    if(RuntimeHUD)RuntimeHUD->AddToViewport();
}
void AAskirPlayerController::TogglePauseMenu(){if(RuntimeHUD)RuntimeHUD->TogglePanel(EAskirUIPanel::Pause);}
void AAskirPlayerController::ToggleInventory(){if(RuntimeHUD)RuntimeHUD->TogglePanel(EAskirUIPanel::Inventory);}
void AAskirPlayerController::ToggleJournal(){if(RuntimeHUD)RuntimeHUD->TogglePanel(EAskirUIPanel::Journal);}
void AAskirPlayerController::ToggleMagic(){if(RuntimeHUD)RuntimeHUD->TogglePanel(EAskirUIPanel::Magic);}
void AAskirPlayerController::CloseAnyMenu()
{
    if(RuntimeHUD)RuntimeHUD->CloseMenu();
    else
    {
        if(auto* Player=Cast<AAskirCharacter>(GetPawn()))
            if(Player->ConversationComponent)Player->ConversationComponent->EndConversation();
        SetMenuInput(false,false);
    }
}
void AAskirPlayerController::ShowNotification(FText Text){if(RuntimeHUD)RuntimeHUD->Notify(Text);}
void AAskirPlayerController::RefreshInventoryPanel(){if(RuntimeHUD)RuntimeHUD->RefreshInventoryPanel();}
bool AAskirPlayerController::OpenElderDialogue(AAskirDornfeldEventActor* Elder){return RuntimeHUD&&RuntimeHUD->ShowElderDialogue(Elder);}
bool AAskirPlayerController::OpenMerchant(AAskirMerchantActor* Merchant){return RuntimeHUD&&RuntimeHUD->ShowMerchant(Merchant);}
bool AAskirPlayerController::OpenCraftingStation(AAskirCraftingStationActor* Station){return RuntimeHUD&&RuntimeHUD->ShowCraftingStation(Station);}
bool AAskirPlayerController::OpenDeliveryDialogue(AAskirDeliveryQuestActor* Giver){return RuntimeHUD&&RuntimeHUD->ShowDeliveryDialogue(Giver);}
bool AAskirPlayerController::OpenConversation(AActor* Speaker,FName DialogueID){return RuntimeHUD&&RuntimeHUD->ShowConversation(Speaker,DialogueID);}
void AAskirPlayerController::SetMenuInput(bool bOpen,bool bPause)
{
    bMenuOpen=bOpen;
    if(bOpen)if(auto* Player=Cast<AAskirCharacter>(GetPawn()))Player->CancelGameplayActions();
    SetPause(bOpen&&bPause);
    ResetIgnoreMoveInput();ResetIgnoreLookInput();
    SetIgnoreMoveInput(bOpen);SetIgnoreLookInput(bOpen);
    bShowMouseCursor=bOpen;
    if(bOpen)
    {
        FInputModeGameAndUI Mode;
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Mode.SetHideCursorDuringCapture(false);
        if(RuntimeHUD)Mode.SetWidgetToFocus(RuntimeHUD->TakeWidget());
        SetInputMode(Mode);
    }
    else SetInputMode(FInputModeGameOnly());
}
