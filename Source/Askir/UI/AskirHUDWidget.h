#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "AskirHUDWidget.generated.h"
class UTextBlock;
class UProgressBar;
class UBorder;
class UVerticalBox;
class AAskirDornfeldEventActor;
class UAskirItemData;
class AAskirCharacter;

UENUM()
enum class EAskirUIPanel : uint8 { None, Pause, Inventory, Journal, Dialogue, Magic };

UCLASS()
class ASKIR_API UAskirSpellButton : public UButton
{
    GENERATED_BODY()
public:
    FName SpellID;
    TWeakObjectPtr<AAskirCharacter> Player;
    UFUNCTION() void SelectSpell();
};

/** Item buttons operate on the live inventory; they keep no copied quantities. */
UCLASS()
class ASKIR_API UAskirInventoryButton : public UButton
{
    GENERATED_BODY()
public:
    UPROPERTY(Transient) TObjectPtr<UAskirItemData> Item;
    TWeakObjectPtr<AAskirCharacter> Player;
    UFUNCTION() void ActivateItem();
};

/** Native UMG HUD: does not require missing Widget Blueprint assets. */
UCLASS()
class ASKIR_API UAskirHUDWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& Geometry,float DeltaSeconds) override;
    void TogglePanel(EAskirUIPanel NewPanel);
    bool ShowElderDialogue(AAskirDornfeldEventActor* Elder);
    void Notify(FText Text);
    void RefreshInventoryPanel();
    void CloseMenu(){ClosePanel();}
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
private:
    void RefreshHUD();
    void BuildPanel();
    UFUNCTION() void ClosePanel();
    UFUNCTION() void SaveClicked();
    UFUNCTION() void LoadClicked();
    UFUNCTION() void QuitClicked();
    UFUNCTION() void ConfirmDialogue();
    UTextBlock* AddText(UVerticalBox* Box,const FString& Text,int32 Size=18);
    UButton* AddButton(UVerticalBox* Box,const FText& Text);
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Stats;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Objective;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Prompt;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Notification;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> Health;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> Stamina;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> Mana;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> SelectedSpell;
    UPROPERTY(Transient) TObjectPtr<UBorder> Menu;
    UPROPERTY(Transient) TObjectPtr<UVerticalBox> MenuBody;
    TWeakObjectPtr<AAskirDornfeldEventActor> CurrentElder;
    EAskirUIPanel Panel=EAskirUIPanel::None;
    float RefreshElapsed=0.f;
    float NotificationSeconds=0.f;
};
