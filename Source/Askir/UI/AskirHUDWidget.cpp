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
#include "Mission/AskirDeliveryMissionComponent.h"
#include "Mission/AskirDeliveryQuestActor.h"
#include "Social/AskirReputationComponent.h"
#include "Travel/AskirWorldTravelSubsystem.h"
#include "Engine/GameInstance.h"
#include "Progression/AskirProgressionComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "Economy/AskirMerchantActor.h"
#include "Economy/AskirMerchantComponent.h"
#include "Economy/AskirTradeBridgeComponent.h"
#include "Crafting/AskirCraftingStationActor.h"
#include "Crafting/AskirCraftingComponent.h"
#include "Game/AskirGameMode.h"
#include "Game/AskirPlayerController.h"
#include "Interaction/AskirInteractable.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"

namespace
{
    UAskirWorldTravelSubsystem* TravelState(const UUserWidget* Widget)
    {
        auto* Instance=Widget&&Widget->GetWorld()?Widget->GetWorld()->GetGameInstance():nullptr;
        return Instance?Instance->GetSubsystem<UAskirWorldTravelSubsystem>():nullptr;
    }
}
void UAskirNarrativeButton::Activate(){if(HUD.IsValid())HUD->ExecuteNarrativeAction(ActionID,ContextGeneration);}
void UAskirEconomyButton::Activate()
{
    if(HUD.IsValid())HUD->ExecuteEconomyAction(EntryID,bSell,bCraft,ContextGeneration);
}
void UAskirInventoryButton::ActivateItem()
{
    AAskirCharacter* Pawn=Player.Get();
    if(!Pawn||!HUD.IsValid()||!HUD->IsActionContextCurrent(ContextGeneration,EAskirUIPanel::Inventory,Pawn)||!Item||Pawn->IsDead()||!Pawn->InventoryComponent)return;
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
    if(Pawn&&HUD.IsValid()&&HUD->IsActionContextCurrent(ContextGeneration,EAskirUIPanel::Magic,Pawn)&&!Pawn->IsDead()&&Pawn->MagicComponent&&Pawn->MagicComponent->SelectSpell(SpellID))
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
void UAskirHUDWidget::AddEconomyButton(FName ID,const FString& Label,bool bSell,bool bCraft,bool bEnabled)
{
    auto* Button=WidgetTree->ConstructWidget<UAskirEconomyButton>();
    Button->HUD=this;Button->EntryID=ID;Button->bSell=bSell;Button->bCraft=bCraft;
    Button->ContextGeneration=ContextGeneration;
    auto* Text=WidgetTree->ConstructWidget<UTextBlock>();Text->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",18));
    Text->SetText(FText::FromString(Label));Button->AddChild(Text);Button->SetIsEnabled(bEnabled);
    Button->OnClicked.AddDynamic(Button,&UAskirEconomyButton::Activate);MenuBody->AddChildToVerticalBox(Button);
}
void UAskirHUDWidget::AddNarrativeButton(FName ID,const FText& Text)
{
    auto* Button=WidgetTree->ConstructWidget<UAskirNarrativeButton>();
    Button->HUD=this;Button->ActionID=ID;Button->ContextGeneration=ContextGeneration;
    auto* Label=WidgetTree->ConstructWidget<UTextBlock>();Label->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",18));
    Label->SetAutoWrapText(true);Label->SetText(Text);Button->AddChild(Label);
    Button->OnClicked.AddDynamic(Button,&UAskirNarrativeButton::Activate);MenuBody->AddChildToVerticalBox(Button);
}
TSharedRef<SWidget> UAskirHUDWidget::RebuildWidget()
{
    if(WidgetTree&&WidgetTree->RootWidget)return Super::RebuildWidget();
    if(!WidgetTree)WidgetTree=NewObject<UWidgetTree>(this,TEXT("WidgetTree"));
    UCanvasPanel* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Canvas;
    UBorder* StatusBackground=WidgetTree->ConstructWidget<UBorder>();
    StatusBackground->SetBrushColor(FLinearColor(.02f,.025f,.035f,.86f));StatusBackground->SetPadding(FMargin(16.f));
    auto* Slot=Canvas->AddChildToCanvas(StatusBackground);Slot->SetPosition(FVector2D(24,24));Slot->SetSize(FVector2D(500,380));
    UVerticalBox* StatusBox=WidgetTree->ConstructWidget<UVerticalBox>();StatusBackground->SetContent(StatusBox);
    RegionTitle=AddText(StatusBox,TEXT("ASKIR"),24);
    Stats=AddText(StatusBox,TEXT(""),16);
    Health=WidgetTree->ConstructWidget<UProgressBar>();Health->SetFillColorAndOpacity(FLinearColor(.7f,.12f,.1f));StatusBox->AddChildToVerticalBox(Health);
    Stamina=WidgetTree->ConstructWidget<UProgressBar>();Stamina->SetFillColorAndOpacity(FLinearColor(.18f,.65f,.3f));StatusBox->AddChildToVerticalBox(Stamina);
    Mana=WidgetTree->ConstructWidget<UProgressBar>();Mana->SetFillColorAndOpacity(FLinearColor(.1f,.4f,.9f));StatusBox->AddChildToVerticalBox(Mana);
    SelectedSpell=AddText(StatusBox,TEXT(""),16);
    Objective=AddText(StatusBox,TEXT(""),18);
    DeliveryObjective=AddText(StatusBox,TEXT(""),18);
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
void UAskirHUDWidget::RefreshInventoryPanel(){if(Panel==EAskirUIPanel::Inventory||Panel==EAskirUIPanel::Merchant||Panel==EAskirUIPanel::Crafting)BuildPanel();}
void UAskirHUDWidget::NativeTick(const FGeometry& Geometry,float DeltaSeconds)
{
    Super::NativeTick(Geometry,DeltaSeconds);
    RefreshElapsed+=DeltaSeconds;
    if(RefreshElapsed>=.1f){RefreshElapsed=0;RefreshHUD();}
    if(NotificationSeconds>0){NotificationSeconds-=DeltaSeconds;if(NotificationSeconds<=0&&Notification)Notification->SetText(FText::GetEmpty());}
    auto* Pawn=Cast<AAskirCharacter>(GetOwningPlayerPawn());
    if((Panel==EAskirUIPanel::Merchant&&(!CurrentMerchant.IsValid()||!CurrentMerchant->CanInteract(Pawn)))||
       (Panel==EAskirUIPanel::Crafting&&(!CurrentStation.IsValid()||!CurrentStation->CanInteract(Pawn)))||
       (Panel==EAskirUIPanel::Delivery&&(!CurrentDeliveryGiver.IsValid()||!CurrentDeliveryGiver->CanInteractWith(Pawn))))ClosePanel();
    if(const auto* State=TravelState(this))
        if(State->bArrivalFailed&&Panel!=EAskirUIPanel::TravelRecovery&&MenuBody)
        {
            ClosePanel();Panel=EAskirUIPanel::TravelRecovery;BuildPanel();Menu->SetVisibility(ESlateVisibility::Visible);
            if(auto* PC=Cast<AAskirPlayerController>(GetOwningPlayer()))PC->SetMenuInput(true,false);
        }
}
void UAskirHUDWidget::RefreshHUD()
{
    auto* Pawn=Cast<AAskirCharacter>(GetOwningPlayerPawn());if(!Pawn||!Stats)return;
    const FString Map=UGameplayStatics::GetCurrentLevelName(this,true);
    if(RegionTitle)RegionTitle->SetText(FText::FromString(Map.StartsWith(TEXT("Dornfelder_Umland"))?TEXT("ASKIR · Dornfelder Umland"):Map.StartsWith(TEXT("Dornfeld_"))?TEXT("ASKIR · Dornfeld"):TEXT("ASKIR")));
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
    DeliveryObjective->SetText(Pawn->DeliveryMission&&Pawn->DeliveryMission->GetStage()!=EAskirDeliveryStage::NotStarted&&Pawn->DeliveryMission->GetStage()!=EAskirDeliveryStage::Completed?Pawn->DeliveryMission->GetObjectiveText():FText::GetEmpty());
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
    if(const auto* State=TravelState(this))if(State->HasPendingTravel())return;
    if(Panel!=EAskirUIPanel::None){ClosePanel();return;}
    Panel=NewPanel;BuildPanel();Menu->SetVisibility(ESlateVisibility::Visible);
    if(auto* PC=Cast<AAskirPlayerController>(GetOwningPlayer()))PC->SetMenuInput(true,Panel==EAskirUIPanel::Pause);
}
void UAskirHUDWidget::BuildPanel()
{
    ++ContextGeneration;
    MenuBody->ClearChildren();auto* Pawn=Cast<AAskirCharacter>(GetOwningPlayerPawn());
    if(Panel==EAskirUIPanel::Pause)
    {
        AddText(MenuBody,TEXT("Pause"),28);
        AddNarrativeButton(TEXT("pause_save"),NSLOCTEXT("AskirUI","Save","Speichern"));
        AddNarrativeButton(TEXT("pause_load"),NSLOCTEXT("AskirUI","Load","Laden"));
        AddNarrativeButton(TEXT("quit"),NSLOCTEXT("AskirUI","Quit","Spiel beenden"));
    }
    else if(Panel==EAskirUIPanel::Inventory&&Pawn&&Pawn->InventoryComponent)
    {
        AddText(MenuBody,TEXT("Inventar"),28);
        if(Pawn->EquipmentComponent)
            AddText(MenuBody,FString::Printf(TEXT("Waffe: %s · Rüstung: %s · Zustand %.0f%%"),*Pawn->EquipmentComponent->EquippedWeaponID.ToString(),*Pawn->EquipmentComponent->EquippedArmorID.ToString(),Pawn->EquipmentComponent->Durability),16);
        for(const auto& Entry:Pawn->InventoryComponent->Items)
        {
            if(!Entry.Item)continue;
            auto* Button=WidgetTree->ConstructWidget<UAskirInventoryButton>();Button->Item=Entry.Item;Button->Player=Pawn;Button->HUD=this;Button->ContextGeneration=ContextGeneration;
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
        if(Pawn->DeliveryMission)
        {
            AddText(MenuBody,TEXT("Die verlorene Lieferung"),22);
            AddText(MenuBody,Pawn->DeliveryMission->GetObjectiveText().ToString());
            for(const auto& Goal:Pawn->QuestComponent->GetObjectivesForQuest(FName(TEXT("SQ_001"))))
                AddText(MenuBody,FString::Printf(TEXT("%s  %d/%d"),*Goal.Description.ToString(),Goal.CurrentCount,Goal.RequiredCount));
            const FName Choice=Pawn->DeliveryMission->GetAcceptedChoice();
            if(!Choice.IsNone())AddText(MenuBody,Choice==FName(TEXT("help"))?TEXT("Entscheidung: Hilfsbereitschaft · Waldhüter +2"):TEXT("Entscheidung: Fairer Handel · Freie Händler +2"),16);
        }
        if(Pawn->ReputationComponent)
        {
            AddText(MenuBody,TEXT("Fraktionen"),22);
            AddText(MenuBody,FString::Printf(TEXT("Waldhüter: %d · Freie Händler: %d"),Pawn->ReputationComponent->GetReputation(TEXT("Waldhüter")),Pawn->ReputationComponent->GetReputation(TEXT("Freie Händler"))),16);
        }
    }
    else if(Panel==EAskirUIPanel::Magic&&Pawn&&Pawn->MagicComponent)
    {
        AddText(MenuBody,TEXT("Magie"),28);AddText(MenuBody,TEXT("Wähle einen Zauber. Mit F wirken; Mausrad wechselt die Auswahl."),16);
        for(const auto& Spell:AskirMagicRules::Catalog)
        {
            auto* Button=WidgetTree->ConstructWidget<UAskirSpellButton>();Button->Player=Pawn;Button->SpellID=FName(UTF8_TO_TCHAR(Spell.ID));Button->HUD=this;Button->ContextGeneration=ContextGeneration;
            auto* Label=WidgetTree->ConstructWidget<UTextBlock>();Label->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",18));
            Label->SetText(FText::FromString(FString::Printf(TEXT("%s · %.0f Mana · %.1fs"),UTF8_TO_TCHAR(Spell.Name),Spell.ManaCost,Spell.Cooldown)));
            Button->AddChild(Label);Button->OnClicked.AddDynamic(Button,&UAskirSpellButton::SelectSpell);MenuBody->AddChildToVerticalBox(Button);
        }
    }
    else if(Panel==EAskirUIPanel::Merchant&&Pawn&&CurrentMerchant.IsValid()&&Pawn->TradeBridgeComponent&&Pawn->InventoryComponent&&Pawn->TradeComponent)
    {
        auto* Shop=CurrentMerchant->Merchant.Get();
        if(!IsValid(Shop)){ClosePanel();return;}
        AddText(MenuBody,CurrentMerchant->MerchantName.ToString(),28);
        AddText(MenuBody,FString::Printf(TEXT("Dein Gold: %d · Händlergold: %d"),Pawn->TradeComponent->Gold,Shop->MerchantGold));
        AddText(MenuBody,TEXT("Kaufen"),22);
        for(const auto& Offer:Shop->Inventory)
        {
            auto* Item=Pawn->TradeBridgeComponent->FindItem(Offer.ItemID);if(!Item)continue;
            const int32 Price=Shop->GetUnitBuyPrice(Offer.ItemID);
            AddEconomyButton(Offer.ItemID,FString::Printf(TEXT("%s · %d Gold · Vorrat %d · Kaufen × 1"),*Item->DisplayName.ToString(),Price,Offer.Stock),false,false,
                Pawn->TradeBridgeComponent->CanBuyItem(Shop,Pawn->InventoryComponent,Offer.ItemID,1,Pawn->TradeComponent->Gold));
        }
        AddText(MenuBody,TEXT("Verkaufen"),22);TSet<FName> Shown;
        for(const auto& Entry:Pawn->InventoryComponent->Items)
        {
            if(!Entry.Item||Shown.Contains(Entry.Item->ItemID)||Pawn->TradeBridgeComponent->FindItem(Entry.Item->ItemID)!=Entry.Item)continue;
            Shown.Add(Entry.Item->ItemID);const int32 Price=Shop->GetUnitSellPrice(Entry.Item->ItemID);
            if(Price<=0)continue;
            AddEconomyButton(Entry.Item->ItemID,FString::Printf(TEXT("%s · %d Gold · Besitzt %d · Verkaufen × 1"),*Entry.Item->DisplayName.ToString(),Price,Pawn->InventoryComponent->GetItemCount(Entry.Item)),true,false,
                Pawn->TradeBridgeComponent->CanSellItem(Shop,Pawn->InventoryComponent,Entry.Item->ItemID,1,Pawn->TradeComponent->Gold));
        }
    }
    else if(Panel==EAskirUIPanel::Crafting&&Pawn&&CurrentStation.IsValid()&&Pawn->CraftingComponent&&Pawn->InventoryComponent)
    {
        AddText(MenuBody,CurrentStation->StationName.ToString(),28);
        AddText(MenuBody,FString::Printf(TEXT("Handwerksstufe %d · Hergestellt %d"),Pawn->CraftingComponent->CraftSkill,Pawn->CraftingComponent->CraftedItems));
        bool bHasRecipe=false;
        for(const auto& Recipe:Pawn->CraftingComponent->Recipes)
        {
            if(!Pawn->CraftingComponent->IsRecipeKnown(Recipe.RecipeID)||!Recipe.Result.Item)continue;
            bHasRecipe=true;AddText(MenuBody,Recipe.Result.Item->DisplayName.ToString(),22);
            for(const auto& Ingredient:Recipe.Ingredients)
                if(Ingredient.Item)AddText(MenuBody,FString::Printf(TEXT("%s · benötigt %d · vorhanden %d"),*Ingredient.Item->DisplayName.ToString(),Ingredient.Quantity,Pawn->InventoryComponent->GetItemCount(Ingredient.Item)),16);
            AddEconomyButton(Recipe.RecipeID,FString::Printf(TEXT("%s × %d herstellen · Stufe %d"),*Recipe.Result.Item->DisplayName.ToString(),Recipe.Result.Quantity,Recipe.RequiredSkill),false,true,
                Pawn->CraftingComponent->CanCraftRecipe(Pawn->InventoryComponent,Recipe.RecipeID));
        }
        if(!bHasRecipe)AddText(MenuBody,TEXT("Du kennst noch keine Rezepte für diese Werkbank."));
    }
    else if(Panel==EAskirUIPanel::Delivery&&Pawn&&Pawn->DeliveryMission&&CurrentDeliveryGiver.IsValid())
    {
        AddText(MenuBody,TEXT("Torben · Die verlorene Lieferung"),28);
        const auto Stage=Pawn->DeliveryMission->GetStage();
        if(Stage==EAskirDeliveryStage::NotStarted)
        {
            if(!bDeliveryDetails)
            {
                AddText(MenuBody,TEXT("Mein Wagen steht noch draußen am Weg. Zwei Banditen haben die Lieferung geraubt. Ohne sie gehen hier bald die Vorräte aus."));
                AddNarrativeButton(TEXT("delivery_more"),NSLOCTEXT("AskirUI","DeliveryMore","Erzähl mir, was geschehen ist."));
            }
            else
            {
                AddText(MenuBody,TEXT("Sie lagern im Dornfelder Umland. Besiege die beiden Banditen, sichere die Waren und kehre zu mir zurück. Ich zahle dir 16 Gold und gebe dir einen Teil der Vorräte."));
                AddText(MenuBody,TEXT("Belohnung: 100 Erfahrung, 16 Gold und eine Handwerkszutat."),16);
                AddNarrativeButton(TEXT("delivery_help"),NSLOCTEXT("AskirUI","DeliveryHelp","Ich helfe dir. Die Wege müssen sicher bleiben. (Waldhüter +2)"));
                AddNarrativeButton(TEXT("delivery_bargain"),NSLOCTEXT("AskirUI","DeliveryBargain","Einverstanden, wenn der Lohn fair bleibt. (Freie Händler +2)"));
            }
        }
        else if(Stage==EAskirDeliveryStage::ReturnToTorben)
        {
            AddText(MenuBody,TEXT("Du hast die Waren gefunden! Jetzt können die Händler wieder durch Dornfeld ziehen. Hier ist dein versprochener Lohn."));
            AddNarrativeButton(TEXT("delivery_turnin"),NSLOCTEXT("AskirUI","DeliveryTurnIn","Die Lieferung übergeben und den Auftrag abschließen."));
        }
        else AddText(MenuBody,Stage==EAskirDeliveryStage::Completed?TEXT("Dank dir sind unsere Vorräte zurück. Dornfeld steht in deiner Schuld."):Pawn->DeliveryMission->GetObjectiveText().ToString());
    }
    else if(Panel==EAskirUIPanel::TravelRecovery)
    {
        AddText(MenuBody,TEXT("Reise unterbrochen"),28);
        AddText(MenuBody,TEXT("Die Ankunft konnte nicht abgeschlossen werden. Dein Fortschritt wird für einen erneuten Versuch aufbewahrt."));
        AddNarrativeButton(TEXT("travel_retry"),NSLOCTEXT("AskirUI","TravelRetry","Erneut versuchen"));
        if(const auto* GM=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))
            if(GM->CanReturnToSavedWorld())AddNarrativeButton(TEXT("travel_return"),NSLOCTEXT("AskirUI","TravelReturnSaved","Zum gesicherten Ort zurückkehren"));
        AddNarrativeButton(TEXT("quit"),NSLOCTEXT("AskirUI","Quit","Spiel beenden"));
        return;
    }
    AddNarrativeButton(TEXT("menu_close"),NSLOCTEXT("AskirUI","Close","Zurück"));
}
bool UAskirHUDWidget::ShowDeliveryDialogue(AAskirDeliveryQuestActor* Giver)
{
    if(const auto* State=TravelState(this))if(State->HasPendingTravel())return false;
    auto* Pawn=Cast<AAskirCharacter>(GetOwningPlayerPawn());
    if(!IsValid(Giver)||!Giver->CanInteractWith(Pawn)||!Pawn->DeliveryMission||!MenuBody)return false;
    ClosePanel();CurrentDeliveryGiver=Giver;Panel=EAskirUIPanel::Delivery;BuildPanel();Menu->SetVisibility(ESlateVisibility::Visible);
    if(auto* PC=Cast<AAskirPlayerController>(GetOwningPlayer()))PC->SetMenuInput(true,false);
    return true;
}
void UAskirHUDWidget::ExecuteNarrativeAction(FName ActionID,uint64 ExpectedGeneration)
{
    auto* Pawn=Cast<AAskirCharacter>(GetOwningPlayerPawn());
    if(!IsActionContextCurrent(ExpectedGeneration,Panel,Pawn))return;
    if(ActionID==FName(TEXT("quit"))&&(Panel==EAskirUIPanel::Pause||Panel==EAskirUIPanel::TravelRecovery)){QuitClicked();return;}
    if(ActionID==FName(TEXT("menu_close"))&&Panel!=EAskirUIPanel::TravelRecovery){ClosePanel();return;}
    if(Panel==EAskirUIPanel::Pause)
    {
        if(ActionID==FName(TEXT("pause_save")))SaveClicked();
        else if(ActionID==FName(TEXT("pause_load")))LoadClicked();
        return;
    }
    if(Panel==EAskirUIPanel::TravelRecovery)
    {
        const auto* State=TravelState(this);if(!State||!State->bArrivalFailed)return;
        auto* GM=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this));if(!GM)return;
        bool bOK=false;
        if(ActionID==FName(TEXT("travel_retry")))bOK=GM->RetryPendingArrival();
        else if(ActionID==FName(TEXT("travel_return")))bOK=GM->ReturnToSavedWorld();
        if(bOK){ClosePanel();Notify(NSLOCTEXT("AskirUI","TravelResuming","Die Reise wird fortgesetzt."));}
        else Notify(NSLOCTEXT("AskirUI","TravelRetryFailed","Die Reise kann noch nicht fortgesetzt werden. Versuche es erneut oder kehre zurück."));
        return;
    }
    if(Panel==EAskirUIPanel::Dialogue&&ActionID==FName(TEXT("mara_confirm"))){ConfirmDialogue();return;}
    if(Panel!=EAskirUIPanel::Delivery||!Pawn->DeliveryMission||!CurrentDeliveryGiver.IsValid()||!CurrentDeliveryGiver->CanInteractWith(Pawn))return;
    if(ActionID==FName(TEXT("delivery_more"))&&Pawn->DeliveryMission->GetStage()==EAskirDeliveryStage::NotStarted)
    {bDeliveryDetails=true;BuildPanel();return;}
    bool bOK=false;
    if(ActionID==FName(TEXT("delivery_help")))bOK=Pawn->DeliveryMission->AcceptDelivery(TEXT("help"));
    else if(ActionID==FName(TEXT("delivery_bargain")))bOK=Pawn->DeliveryMission->AcceptDelivery(TEXT("bargain"));
    else if(ActionID==FName(TEXT("delivery_turnin")))bOK=Pawn->DeliveryMission->ReturnToTorben();
    if(bOK){ClosePanel();Notify(NSLOCTEXT("AskirUI","DeliveryUpdated","Die verlorene Lieferung: Auftrag aktualisiert."));}
    else {Notify(Pawn->DeliveryMission->GetLastFailureText());BuildPanel();}
}
bool UAskirHUDWidget::ShowMerchant(AAskirMerchantActor* Merchant)
{
    if(const auto* State=TravelState(this))if(State->HasPendingTravel())return false;
    auto* Pawn=Cast<AAskirCharacter>(GetOwningPlayerPawn());
    if(!IsValid(Merchant)||!Merchant->CanInteract(Pawn)||!MenuBody||!Pawn->TradeBridgeComponent||!Pawn->InventoryComponent||!Pawn->TradeComponent)return false;
    ClosePanel();CurrentMerchant=Merchant;Panel=EAskirUIPanel::Merchant;BuildPanel();Menu->SetVisibility(ESlateVisibility::Visible);
    if(auto* PC=Cast<AAskirPlayerController>(GetOwningPlayer()))PC->SetMenuInput(true,false);
    return true;
}
bool UAskirHUDWidget::ShowCraftingStation(AAskirCraftingStationActor* Station)
{
    if(const auto* State=TravelState(this))if(State->HasPendingTravel())return false;
    auto* Pawn=Cast<AAskirCharacter>(GetOwningPlayerPawn());
    if(!IsValid(Station)||!Station->CanInteract(Pawn)||!MenuBody||!Pawn->InventoryComponent)return false;
    ClosePanel();CurrentStation=Station;Panel=EAskirUIPanel::Crafting;BuildPanel();Menu->SetVisibility(ESlateVisibility::Visible);
    if(auto* PC=Cast<AAskirPlayerController>(GetOwningPlayer()))PC->SetMenuInput(true,false);
    return true;
}
void UAskirHUDWidget::ExecuteEconomyAction(FName EntryID,bool bSell,bool bCraft,uint64 ExpectedGeneration)
{
    auto* Pawn=Cast<AAskirCharacter>(GetOwningPlayerPawn());
    if(!IsActionContextCurrent(ExpectedGeneration,Panel,Pawn))return;
    if(!Pawn||Pawn->IsDead()||!Pawn->InventoryComponent)return;
    bool bOK=false;
    if(bCraft&&Panel==EAskirUIPanel::Crafting&&CurrentStation.IsValid()&&CurrentStation->CanInteract(Pawn))
        bOK=Pawn->CraftingComponent->CraftRecipe(Pawn->InventoryComponent,EntryID);
    else if(!bCraft&&Panel==EAskirUIPanel::Merchant&&CurrentMerchant.IsValid()&&CurrentMerchant->CanInteract(Pawn)&&Pawn->TradeBridgeComponent&&Pawn->TradeComponent)
    {
        auto* Shop=CurrentMerchant->Merchant.Get();
        bOK=bSell?Pawn->TradeBridgeComponent->SellItem(Shop,Pawn->InventoryComponent,EntryID,1,Pawn->TradeComponent->Gold):
                  Pawn->TradeBridgeComponent->BuyItem(Shop,Pawn->InventoryComponent,EntryID,1,Pawn->TradeComponent->Gold);
    }
    Notify(bOK?NSLOCTEXT("AskirUI","TransactionDone","Vorgang abgeschlossen."):NSLOCTEXT("AskirUI","TransactionFailed","Vorgang nicht möglich. Prüfe Reichweite, Vorrat, Gold und Materialien."));
    if(Panel==EAskirUIPanel::Merchant||Panel==EAskirUIPanel::Crafting)BuildPanel();
}
bool UAskirHUDWidget::IsActionContextCurrent(uint64 ExpectedGeneration,EAskirUIPanel ExpectedPanel,AAskirCharacter* Player) const
{
    if(ContextGeneration!=ExpectedGeneration||Panel!=ExpectedPanel||!IsValid(GetOwningPlayer()))return false;
    if(ExpectedPanel!=EAskirUIPanel::TravelRecovery)
    {
        if(!IsValid(Player)||Player!=GetOwningPlayerPawn())return false;
        if(const auto* State=TravelState(this))if(State->HasPendingTravel())return false;
    }
    if(const auto* GM=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))if(GM->IsPersistenceBusy())return false;
    return true;
}
bool UAskirHUDWidget::ShowElderDialogue(AAskirDornfeldEventActor* Elder)
{
    if(const auto* State=TravelState(this))if(State->HasPendingTravel())return false;
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
        AddNarrativeButton(TEXT("mara_confirm"),NSLOCTEXT("AskirUI","Accept","Ich werde dem Flüstern nachgehen."));
    }
    else if(Stage==EAskirDornfeldStage::ReturnToElder)
    {
        AddText(MenuBody,TEXT("Die Schatten sind verstummt. Du hast Dornfeld Hoffnung gebracht. Nimm diese Belohnung für deinen Mut."));
        AddNarrativeButton(TEXT("mara_confirm"),NSLOCTEXT("AskirUI","TurnIn","Auftrag abschließen"));
    }
    else AddText(MenuBody,Stage==EAskirDornfeldStage::Completed?TEXT("Dornfeld wird deine Hilfe nicht vergessen."):Pawn->DornfeldMission->GetObjectiveText().ToString());
    AddNarrativeButton(TEXT("menu_close"),NSLOCTEXT("AskirUI","Leave","Gespräch beenden"));
    Menu->SetVisibility(ESlateVisibility::Visible);
    if(auto* PC=Cast<AAskirPlayerController>(GetOwningPlayer()))PC->SetMenuInput(true,false);
    return true;
}
void UAskirHUDWidget::ClosePanel()
{
    ++ContextGeneration;
    Panel=EAskirUIPanel::None;CurrentElder.Reset();CurrentMerchant.Reset();CurrentStation.Reset();CurrentDeliveryGiver.Reset();bDeliveryDetails=false;if(Menu)Menu->SetVisibility(ESlateVisibility::Collapsed);
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
