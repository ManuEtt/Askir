#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Settings/AskirGameUserSettings.h"
#include "AskirHUDWidget.generated.h"
class UTextBlock;
class UProgressBar;
class UBorder;
class UVerticalBox;
class UCanvasPanelSlot;
class AAskirDornfeldEventActor;
class UAskirItemData;
class AAskirCharacter;
class AAskirMerchantActor;
class AAskirCraftingStationActor;
class AAskirDeliveryQuestActor;
class UAskirHUDWidget;

UENUM()
enum class EAskirUIPanel : uint8 { None, Pause, Inventory, Journal, Dialogue, Magic, Merchant, Crafting, TravelRecovery, Title, ConfirmNewGame, Settings, VideoConfirm, FreshStart, Conversation };

UCLASS()
class ASKIR_API UAskirNarrativeButton : public UButton
{
    GENERATED_BODY()
public:
    TWeakObjectPtr<UAskirHUDWidget> HUD;
    FName ActionID;
    uint64 ContextGeneration=0;
    UFUNCTION() void Activate();
};

/** Stable backend choice identity and revision protect against stale dialogue buttons. */
UCLASS()
class ASKIR_API UAskirConversationButton : public UButton
{
    GENERATED_BODY()
public:
    TWeakObjectPtr<UAskirHUDWidget> HUD;
    FName ChoiceID;
    int64 ConversationRevision=0;
    uint64 ContextGeneration=0;
    UFUNCTION() void Activate();
};

/** Actions revalidate the live menu context; no copied inventory or wallet is committed. */
UCLASS()
class ASKIR_API UAskirEconomyButton : public UButton
{
    GENERATED_BODY()
public:
    TWeakObjectPtr<UAskirHUDWidget> HUD;
    FName EntryID;
    bool bSell=false;
    bool bCraft=false;
    uint64 ContextGeneration=0;
    UFUNCTION() void Activate();
};

UCLASS()
class ASKIR_API UAskirSpellButton : public UButton
{
    GENERATED_BODY()
public:
    FName SpellID;
    TWeakObjectPtr<AAskirCharacter> Player;
    TWeakObjectPtr<UAskirHUDWidget> HUD;
    uint64 ContextGeneration=0;
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
    TWeakObjectPtr<UAskirHUDWidget> HUD;
    uint64 ContextGeneration=0;
    UFUNCTION() void ActivateItem();
};

/** Native UMG HUD: does not require missing Widget Blueprint assets. */
UCLASS()
class ASKIR_API UAskirHUDWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& Geometry,float DeltaSeconds) override;
    void TogglePanel(EAskirUIPanel NewPanel);
    bool ShowElderDialogue(AAskirDornfeldEventActor* Elder);
    bool ShowMerchant(AAskirMerchantActor* Merchant);
    bool ShowCraftingStation(AAskirCraftingStationActor* Station);
    bool ShowDeliveryDialogue(AAskirDeliveryQuestActor* Giver);
    bool ShowConversation(AActor* Speaker,FName DialogueID=NAME_None);
    void ExecuteConversationChoice(FName ChoiceID,int64 ExpectedRevision,uint64 ExpectedGeneration);
    void ExecuteNarrativeAction(FName ActionID,uint64 ExpectedGeneration);
    void ExecuteEconomyAction(FName EntryID,bool bSell,bool bCraft,uint64 ExpectedGeneration);
    bool IsActionContextCurrent(uint64 ExpectedGeneration,EAskirUIPanel ExpectedPanel,AAskirCharacter* Player) const;
    void Notify(FText Text);
    void RefreshInventoryPanel();
    void CloseMenu(){ClosePanel();}
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual FReply NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
private:
    void RefreshHUD();
    void RefreshTargetHUD(const FGeometry& Geometry);
    void HideTargetHUD();
    void BuildPanel();
    void BuildConversationPanel();
    void UpdateFrontend();
    void ShowTitle();
    void OpenSettings();
    void BuildSettingsPanel();
    void ExecuteFrontendAction(FName ActionID);
    bool IsFrontendPanel() const;
    UFUNCTION() void ClosePanel();
    UFUNCTION() void SaveClicked();
    UFUNCTION() void LoadClicked();
    UFUNCTION() void QuitClicked();
    UFUNCTION() void ConfirmDialogue();
    UTextBlock* AddText(UVerticalBox* Box,const FString& Text,int32 Size=18);
    UButton* AddButton(UVerticalBox* Box,const FText& Text);
    void AddEconomyButton(FName ID,const FString& Label,bool bSell,bool bCraft,bool bEnabled);
    void AddNarrativeButton(FName ID,const FText& Text,bool bEnabled=true);
    UPROPERTY(Transient) TObjectPtr<UTextBlock> RegionTitle;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Stats;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Objective;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> DeliveryObjective;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Prompt;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Notification;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> Health;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> Stamina;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> Mana;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> SelectedSpell;
    UPROPERTY(Transient) TObjectPtr<UBorder> TargetPanel;
    UPROPERTY(Transient) TObjectPtr<UCanvasPanelSlot> TargetPanelSlot;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> TargetName;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> TargetVitals;
    UPROPERTY(Transient) TObjectPtr<UProgressBar> TargetHealth;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> TargetMarker;
    UPROPERTY(Transient) TObjectPtr<UCanvasPanelSlot> TargetMarkerSlot;
    UPROPERTY(Transient) TObjectPtr<UBorder> Menu;
    UPROPERTY(Transient) TObjectPtr<UVerticalBox> MenuBody;
    TWeakObjectPtr<AAskirDornfeldEventActor> CurrentElder;
    TWeakObjectPtr<AAskirMerchantActor> CurrentMerchant;
    TWeakObjectPtr<AAskirCraftingStationActor> CurrentStation;
    int64 ConversationRevision=-1;
    EAskirUIPanel Panel=EAskirUIPanel::None;
    EAskirUIPanel SettingsReturnPanel=EAskirUIPanel::Title;
    FAskirSettingsDraft SettingsDraft;
    bool bFrontendInitialized=false;
    bool bCanContinue=false;
    int32 VideoSeconds=-1;
    float RefreshElapsed=0.f;
    float NotificationSeconds=0.f;
    uint64 ContextGeneration=0;
};
