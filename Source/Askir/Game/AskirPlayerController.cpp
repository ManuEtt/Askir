#include "Game/AskirPlayerController.h"
#include "UI/AskirHUDWidget.h"
#include "Characters/AskirCharacter.h"

void AAskirPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if(!IsLocalController()||!GetLocalPlayer())return;
    RuntimeHUD=CreateWidget<UAskirHUDWidget>(this,UAskirHUDWidget::StaticClass());
    if(RuntimeHUD)RuntimeHUD->AddToViewport();
    SetInputMode(FInputModeGameOnly());
}
void AAskirPlayerController::TogglePauseMenu(){if(RuntimeHUD)RuntimeHUD->TogglePanel(EAskirUIPanel::Pause);}
void AAskirPlayerController::ToggleInventory(){if(RuntimeHUD)RuntimeHUD->TogglePanel(EAskirUIPanel::Inventory);}
void AAskirPlayerController::ToggleJournal(){if(RuntimeHUD)RuntimeHUD->TogglePanel(EAskirUIPanel::Journal);}
void AAskirPlayerController::ToggleMagic(){if(RuntimeHUD)RuntimeHUD->TogglePanel(EAskirUIPanel::Magic);}
void AAskirPlayerController::CloseAnyMenu(){if(RuntimeHUD)RuntimeHUD->CloseMenu();else SetMenuInput(false,false);}
void AAskirPlayerController::ShowNotification(FText Text){if(RuntimeHUD)RuntimeHUD->Notify(Text);}
void AAskirPlayerController::RefreshInventoryPanel(){if(RuntimeHUD)RuntimeHUD->RefreshInventoryPanel();}
bool AAskirPlayerController::OpenElderDialogue(AAskirDornfeldEventActor* Elder){return RuntimeHUD&&RuntimeHUD->ShowElderDialogue(Elder);}
bool AAskirPlayerController::OpenMerchant(AAskirMerchantActor* Merchant){return RuntimeHUD&&RuntimeHUD->ShowMerchant(Merchant);}
bool AAskirPlayerController::OpenCraftingStation(AAskirCraftingStationActor* Station){return RuntimeHUD&&RuntimeHUD->ShowCraftingStation(Station);}
bool AAskirPlayerController::OpenDeliveryDialogue(AAskirDeliveryQuestActor* Giver){return RuntimeHUD&&RuntimeHUD->ShowDeliveryDialogue(Giver);}
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
        SetInputMode(Mode);
    }
    else SetInputMode(FInputModeGameOnly());
}
