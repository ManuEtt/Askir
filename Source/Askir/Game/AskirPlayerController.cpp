#include "Game/AskirPlayerController.h"
#include "UI/AskirHUDWidget.h"

void AAskirPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if(!IsLocalController())return;
    RuntimeHUD=CreateWidget<UAskirHUDWidget>(this,UAskirHUDWidget::StaticClass());
    if(RuntimeHUD)RuntimeHUD->AddToViewport();
    SetInputMode(FInputModeGameOnly());
}
void AAskirPlayerController::TogglePauseMenu(){if(RuntimeHUD)RuntimeHUD->TogglePanel(EAskirUIPanel::Pause);}
void AAskirPlayerController::ToggleInventory(){if(RuntimeHUD)RuntimeHUD->TogglePanel(EAskirUIPanel::Inventory);}
void AAskirPlayerController::ToggleJournal(){if(RuntimeHUD)RuntimeHUD->TogglePanel(EAskirUIPanel::Journal);}
void AAskirPlayerController::ShowNotification(FText Text){if(RuntimeHUD)RuntimeHUD->Notify(Text);}
void AAskirPlayerController::RefreshInventoryPanel(){if(RuntimeHUD)RuntimeHUD->RefreshInventoryPanel();}
bool AAskirPlayerController::OpenElderDialogue(AAskirDornfeldEventActor* Elder){return RuntimeHUD&&RuntimeHUD->ShowElderDialogue(Elder);}
void AAskirPlayerController::SetMenuInput(bool bOpen,bool bPause)
{
    bMenuOpen=bOpen;
    SetPause(bOpen&&bPause);
    ResetIgnoreMoveInput();ResetIgnoreLookInput();
    SetIgnoreMoveInput(bOpen);SetIgnoreLookInput(bOpen);
    bShowMouseCursor=bOpen;
    if(bOpen)
    {
        FInputModeGameAndUI Mode;
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Mode.SetHideCursorDuringCapture(false);
        SetInputMode(Mode);
    }
    else SetInputMode(FInputModeGameOnly());
}
