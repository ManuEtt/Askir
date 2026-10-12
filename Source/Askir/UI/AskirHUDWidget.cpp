#include "UI/AskirHUDWidget.h"
#include "Magic/AskirMagicComponent.h"
#include "Magic/AskirSpellCatalog.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Styling/CoreStyle.h"
#include "Characters/AskirCharacter.h"
#include "Combat/AskirCombatComponent.h"
#include "Equipment/AskirEquipmentComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Mission/AskirDornfeldMissionComponent.h"
#include "Mission/AskirDornfeldEventActor.h"
#include "Progression/AskirProgressionComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "Game/AskirGameMode.h"
#include "Game/AskirPlayerController.h"
#include "Interaction/AskirInteractable.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"

void UAskirInventoryButton::ActivateItem()
{
    AAskirCharacter* Pawn=Player.Get();
    if(!Pawn||!Item||Pawn->IsDead()||!Pawn->InventoryComponent)return;
    bool bOK=false;
    if(Item->ItemType==EAskirItemType::Weapon||Item->ItemType==EAskirItemType::Armor)
        bOK=Pawn->EquipmentComponent&&Pawn->EquipmentComponent->TryEquipInventoryItem(Item);
    else bOK=Pawn->InventoryComponent->UseItem(Item);
    if(auto* PC=Cast<AAskirPlayerController>(Pawn->GetController()))
    {
        PC->ShowNotification(bOK?NSLOCTEXT("AskirUI","ItemUsed","Gegenstand verwendet."):NSLOCTEXT("AskirUI","ItemNotUsable","Dieser Gegenstand kann derzeit nicht verwendet werden."));
        PC->RefreshInventoryPanel();
    }
}
void UAskirSpellButton::SelectSpell()
{
    auto* Pawn=Player.Get();
    if(Pawn&&!Pawn->IsDead()&&Pawn->MagicComponent&&Pawn->MagicComponent->SelectSpell(SpellID))
        if(auto* PC=Cast<AAskirPlayerController>(Pawn->GetController())){PC->CloseAnyMenu();PC->ShowNotification(Pawn->MagicComponent->GetSelectedSpellName());}
}
UTextBlock* UAskirHUDWidget::AddText(UVerticalBox* Box,const FString& Text,int32 Size)
{
    UTextBlock* Result=WidgetTree->ConstructWidget<UTextBlock>();
    Result->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",Size));
    Result->SetText(FText::FromString(Text));Result->SetAutoWrapText(true);
    Box->AddChildToVerticalBox(Result);return Result;
}
UButton* UAskirHUDWidget::AddButton(UVerticalBox* Box,const FText& Text)
{
    UButton* Result=WidgetTree->ConstructWidget<UButton>();
    UTextBlock* Label=WidgetTree->ConstructWidget<UTextBlock>();
    Label->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",18));Label->SetText(Text);
    Result->AddChild(Label);Box->AddChildToVerticalBox(Result);return Result;
}
TSharedRef<SWidget> UAskirHUDWidget::RebuildWidget()
{
    if(WidgetTree&&WidgetTree->RootWidget)return Super::RebuildWidget();
    if(!WidgetTree)WidgetTree=NewObject<UWidgetTree>(this,TEXT("WidgetTree"));
    UCanvasPanel* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Canvas;
    UBorder* StatusBackground=WidgetTree->ConstructWidget<UBorder>();
    StatusBackground->SetBrushColor(FLinearColor(.02f,.025f,.035f,.86f));StatusBackground->SetPadding(FMargin(16.f));
    auto* Slot=Canvas->AddChildToCanvas(StatusBackground);Slot->SetPosition(FVector2D(24,24));Slot->SetSize(FVector2D(500,300));
    UVerticalBox* StatusBox=WidgetTree->ConstructWidget<UVerticalBox>();StatusBackground->SetContent(StatusBox);
    AddText(StatusBox,TEXT("ASKIR · Dornfeld"),24);
    Stats=AddText(StatusBox,TEXT(""),16);
    Health=WidgetTree->ConstructWidget<UProgressBar>();Health->SetFillColorAndOpacity(FLinearColor(.7f,.12f,.1f));StatusBox->AddChildToVerticalBox(Health);
    Stamina=WidgetTree->ConstructWidget<UProgressBar>();Stamina->SetFillColorAndOpacity(FLinearColor(.18f,.65f,.3f));StatusBox->AddChildToVerticalBox(Stamina);
    Mana=WidgetTree->ConstructWidget<UProgressBar>();Mana->SetFillColorAndOpacity(FLinearColor(.1f,.4f,.9f));StatusBox->AddChildToVerticalBox(Mana);
    SelectedSpell=AddText(StatusBox,TEXT(""),16);
    Objective=AddText(StatusBox,TEXT(""),18);
    Prompt=WidgetTree->ConstructWidget<UTextBlock>();Prompt->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",20));
    auto* PromptSlot=Canvas->AddChildToCanvas(Prompt);PromptSlot->SetAnchors(FAnchors(.5f,.75f));PromptSlot->SetAlignment(FVector2D(.5f,0));PromptSlot->SetAutoSize(true);
    Notification=WidgetTree->ConstructWidget<UTextBlock>();Notification->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",18));
    auto* NoticeSlot=Canvas->AddChildToCanvas(Notification);NoticeSlot->SetAnchors(FAnchors(.5f,.86f));NoticeSlot->SetAlignment(FVector2D(.5f,0));NoticeSlot->SetAutoSize(true);
    UTextBlock* Hints=WidgetTree->ConstructWidget<UTextBlock>();Hints->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",14));
    Hints->SetText(NSLOCTEXT("AskirUI","ControlsV78","E Interaktion · Q Block · Alt Ausweichen · F Zauber · Mausrad Auswahl · K Magie · Tab Inventar · J Journal · F5/F9 · Esc Pause"));
    auto* HintSlot=Canvas->AddChildToCanvas(Hints);HintSlot->SetAnchors(FAnchors(.5f,.96f));HintSlot->SetAlignment(FVector2D(.5f,0));HintSlot->SetAutoSize(true);
    Menu=WidgetTree->ConstructWidget<UBorder>();Menu->SetBrushColor(FLinearColor(.025f,.03f,.04f,.98f));Menu->SetPadding(FMargin(24));
    auto* MenuSlot=Canvas->AddChildToCanvas(Menu);MenuSlot->SetAnchors(FAnchors(.5f,.5f));MenuSlot->SetAlignment(FVector2D(.5f,.5f));MenuSlot->SetSize(FVector2D(620,520));
    UScrollBox* Scroll=WidgetTree->ConstructWidget<UScrollBox>();Menu->SetContent(Scroll);
    MenuBody=WidgetTree->ConstructWidget<UVerticalBox>();Scroll->AddChild(MenuBody);Menu->SetVisibility(ESlateVisibility::Collapsed);
    return Super::RebuildWidget();
}
void UAskirHUDWidget::NativeConstruct(){Super::NativeConstruct();RefreshHUD();}
void UAskirHUDWidget::RefreshInventoryPanel(){if(Panel==EAskirUIPanel::Inventory)BuildPanel();}
void UAskirHUDWidget::NativeTick(const FGeometry& Geometry,float DeltaSeconds)
{
    Super::NativeTick(Geometry,DeltaSeconds);
    RefreshElapsed+=DeltaSeconds;
    if(RefreshElapsed>=.1f){RefreshElapsed=0;RefreshHUD();}
    if(NotificationSeconds>0){NotificationSeconds-=DeltaSeconds;if(NotificationSeconds<=0&&Notification)Notification->SetText(FText::GetEmpty());}
}
void UAskirHUDWidget::RefreshHUD()
{
    auto* Pawn=Cast<AAskirCharacter>(GetOwningPlayerPawn());if(!Pawn||!Stats)return;
    if(Pawn->CombatComponent)Health->SetPercent(Pawn->CombatComponent->GetHealthPercent());
    Stamina->SetPercent(Pawn->GetStaminaPercent());
    const auto* XP=Pawn->ProgressionComponent.Get();
    Stats->SetText(FText::FromString(FString::Printf(TEXT("LP %.0f · Ausdauer %.0f · Gold %d · Stufe %d · XP %d/%d"),Pawn->CombatComponent?Pawn->CombatComponent->GetHealth():0,Pawn->Stamina,Pawn->TradeComponent?Pawn->TradeComponent->Gold:0,XP?XP->Progress.Level:1,XP?XP->Progress.Experience:0,XP?XP->Progress.Experience+XP->ExperienceToNextLevel():0)));
    if(Pawn->MagicComponent)
    {
        Mana->SetPercent(Pawn->MagicComponent->GetManaPercent());
        SelectedSpell->SetText(FText::FromString(FString::Printf(TEXT("Mana %.0f · %s · Bereit in %.1fs · Schild %.0f"),Pawn->MagicComponent->GetMana(),*Pawn->MagicComponent->GetSelectedSpellName().ToString(),Pawn->MagicComponent->GetSelectedCooldownRemaining(),Pawn->MagicComponent->GetShield())));
    }
    Objective->SetText(Pawn->IsDead()?NSLOCTEXT("AskirUI","Dead","Du bist gefallen. Lade mit F9 einen Spielstand."):Pawn->DornfeldMission?Pawn->DornfeldMission->GetObjectiveText():FText::GetEmpty());
    Prompt->SetText(FText::GetEmpty());
    if(Panel!=EAskirUIPanel::None||Pawn->IsDead())return;
    const FVector Start=Pawn->GetActorLocation()+FVector(0,0,50);
    FHitResult Hit;FCollisionQueryParams Params(SCENE_QUERY_STAT(AskirUIPrompt),false,Pawn);
    if(GetWorld()->LineTraceSingleByChannel(Hit,Start,Start+GetOwningPlayer()->GetControlRotation().Vector()*250.f,ECC_Visibility,Params))
        if(AActor* Target=Hit.GetActor())
            if(Target->GetClass()->ImplementsInterface(UAskirInteractable::StaticClass()))Prompt->SetText(FText::FromString(TEXT("E · ")+IAskirInteractable::Execute_GetInteractionText(Target).ToString()));
}
void UAskirHUDWidget::Notify(FText Text){if(Notification)Notification->SetText(Text);NotificationSeconds=4.f;}
void UAskirHUDWidget::TogglePanel(EAskirUIPanel NewPanel)
{
    if(Panel!=EAskirUIPanel::None){ClosePanel();return;}
    Panel=NewPanel;BuildPanel();Menu->SetVisibility(ESlateVisibility::Visible);
    if(auto* PC=Cast<AAskirPlayerController>(GetOwningPlayer()))PC->SetMenuInput(true,Panel==EAskirUIPanel::Pause);
}
void UAskirHUDWidget::BuildPanel()
{
    MenuBody->ClearChildren();auto* Pawn=Cast<AAskirCharacter>(GetOwningPlayerPawn());
    if(Panel==EAskirUIPanel::Pause)
    {
        AddText(MenuBody,TEXT("Pause"),28);
        AddButton(MenuBody,NSLOCTEXT("AskirUI","Save","Speichern"))->OnClicked.AddDynamic(this,&UAskirHUDWidget::SaveClicked);
        AddButton(MenuBody,NSLOCTEXT("AskirUI","Load","Laden"))->OnClicked.AddDynamic(this,&UAskirHUDWidget::LoadClicked);
        AddButton(MenuBody,NSLOCTEXT("AskirUI","Quit","Spiel beenden"))->OnClicked.AddDynamic(this,&UAskirHUDWidget::QuitClicked);
    }
    else if(Panel==EAskirUIPanel::Inventory&&Pawn&&Pawn->InventoryComponent)
    {
        AddText(MenuBody,TEXT("Inventar"),28);
        for(const auto& Entry:Pawn->InventoryComponent->Items)
        {
            if(!Entry.Item)continue;
            auto* Button=WidgetTree->ConstructWidget<UAskirInventoryButton>();Button->Item=Entry.Item;Button->Player=Pawn;
            UTextBlock* Label=WidgetTree->ConstructWidget<UTextBlock>();Label->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",18));
            Label->SetText(FText::FromString(FString::Printf(TEXT("%s × %d"),*Entry.Item->DisplayName.ToString(),Entry.Quantity)));
            Button->SetToolTipText(Entry.Item->Description);Button->AddChild(Label);Button->OnClicked.AddDynamic(Button,&UAskirInventoryButton::ActivateItem);MenuBody->AddChildToVerticalBox(Button);
        }
        if(Pawn->InventoryComponent->Items.IsEmpty())AddText(MenuBody,TEXT("Dein Inventar ist leer."));
    }
    else if(Panel==EAskirUIPanel::Journal&&Pawn&&Pawn->QuestComponent)
    {
        AddText(MenuBody,TEXT("Questjournal"),28);
        AddText(MenuBody,TEXT("Das Flüstern im Brunnen"),22);
        for(const auto& Goal:Pawn->QuestComponent->GetObjectivesForQuest(FName(TEXT("DORNFELD_BRUNNEN"))))
            AddText(MenuBody,FString::Printf(TEXT("%s  %d/%d"),*Goal.Description.ToString(),Goal.CurrentCount,Goal.RequiredCount));
        if(Pawn->DornfeldMission)AddText(MenuBody,Pawn->DornfeldMission->GetObjectiveText().ToString());
    }
    else if(Panel==EAskirUIPanel::Magic&&Pawn&&Pawn->MagicComponent)
    {
        AddText(MenuBody,TEXT("Magie"),28);AddText(MenuBody,TEXT("Wähle einen Zauber. Mit F wirken; Mausrad wechselt die Auswahl."),16);
        for(const auto& Spell:AskirMagicRules::Catalog)
        {
            auto* Button=WidgetTree->ConstructWidget<UAskirSpellButton>();Button->Player=Pawn;Button->SpellID=FName(UTF8_TO_TCHAR(Spell.ID));
            auto* Label=WidgetTree->ConstructWidget<UTextBlock>();Label->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",18));
            Label->SetText(FText::FromString(FString::Printf(TEXT("%s · %.0f Mana · %.1fs"),UTF8_TO_TCHAR(Spell.Name),Spell.ManaCost,Spell.Cooldown)));
            Button->AddChild(Label);Button->OnClicked.AddDynamic(Button,&UAskirSpellButton::SelectSpell);MenuBody->AddChildToVerticalBox(Button);
        }
    }
    AddButton(MenuBody,NSLOCTEXT("AskirUI","Close","Zurück"))->OnClicked.AddDynamic(this,&UAskirHUDWidget::ClosePanel);
}
bool UAskirHUDWidget::ShowElderDialogue(AAskirDornfeldEventActor* Elder)
{
    auto* Pawn=Cast<AAskirCharacter>(GetOwningPlayerPawn());
    if(!Elder||!Pawn||Pawn->IsDead()||!Pawn->DornfeldMission||!MenuBody)return false;
    if(FVector::DistSquared(Pawn->GetActorLocation(),Elder->GetActorLocation())>FMath::Square(Elder->InteractionRange))return false;
    ClosePanel();CurrentElder=Elder;Panel=EAskirUIPanel::Dialogue;MenuBody->ClearChildren();
    AddText(MenuBody,TEXT("Dorfälteste Mara"),28);
    const auto Stage=Pawn->DornfeldMission->GetStage();
    if(Stage==EAskirDornfeldStage::NotStarted)
    {
        AddText(MenuBody,TEXT("Seit die Krone zerbrach, flüstert der alte Brunnen. Drei Scherben liegen im Dorf verstreut. Bringe sie zusammen, bezwinge die beiden Schattenwächter und öffne die Kellertruhe. Dann kehre zu mir zurück."));
        AddText(MenuBody,TEXT("Belohnung: 90 Gold und 180 Erfahrung."));
        AddButton(MenuBody,NSLOCTEXT("AskirUI","Accept","Ich werde dem Flüstern nachgehen."))->OnClicked.AddDynamic(this,&UAskirHUDWidget::ConfirmDialogue);
    }
    else if(Stage==EAskirDornfeldStage::ReturnToElder)
    {
        AddText(MenuBody,TEXT("Die Schatten sind verstummt. Du hast Dornfeld Hoffnung gebracht. Nimm diese Belohnung für deinen Mut."));
        AddButton(MenuBody,NSLOCTEXT("AskirUI","TurnIn","Auftrag abschließen"))->OnClicked.AddDynamic(this,&UAskirHUDWidget::ConfirmDialogue);
    }
    else AddText(MenuBody,Stage==EAskirDornfeldStage::Completed?TEXT("Dornfeld wird deine Hilfe nicht vergessen."):Pawn->DornfeldMission->GetObjectiveText().ToString());
    AddButton(MenuBody,NSLOCTEXT("AskirUI","Leave","Gespräch beenden"))->OnClicked.AddDynamic(this,&UAskirHUDWidget::ClosePanel);
    Menu->SetVisibility(ESlateVisibility::Visible);
    if(auto* PC=Cast<AAskirPlayerController>(GetOwningPlayer()))PC->SetMenuInput(true,false);
    return true;
}
void UAskirHUDWidget::ClosePanel()
{
    Panel=EAskirUIPanel::None;CurrentElder.Reset();if(Menu)Menu->SetVisibility(ESlateVisibility::Collapsed);
    if(auto* PC=Cast<AAskirPlayerController>(GetOwningPlayer()))PC->SetMenuInput(false,false);
}
void UAskirHUDWidget::ConfirmDialogue()
{
    auto* Pawn=Cast<AAskirCharacter>(GetOwningPlayerPawn());
    const bool bOK=Pawn&&!Pawn->IsDead()&&CurrentElder.IsValid()&&CurrentElder->TriggerEvent(Pawn);
    ClosePanel();Notify(bOK?NSLOCTEXT("AskirUI","QuestUpdated","Auftrag aktualisiert."):NSLOCTEXT("AskirUI","TooFar","Mara ist nicht mehr erreichbar."));
}
void UAskirHUDWidget::SaveClicked(){if(auto* GM=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))Notify(GM->SaveGame(TEXT("AskirQuick"))?NSLOCTEXT("AskirUI","Saved","Spiel gespeichert."):NSLOCTEXT("AskirUI","SaveFailed","Speichern fehlgeschlagen."));}
void UAskirHUDWidget::LoadClicked(){if(auto* GM=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this))){const bool bOK=GM->LoadGame(TEXT("AskirQuick"));ClosePanel();Notify(bOK?NSLOCTEXT("AskirUI","Loaded","Spiel geladen."):NSLOCTEXT("AskirUI","LoadFailed","Laden fehlgeschlagen oder kein Spielstand vorhanden."));}}
void UAskirHUDWidget::QuitClicked(){UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,false);}
