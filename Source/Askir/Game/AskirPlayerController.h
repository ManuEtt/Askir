#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AskirPlayerController.generated.h"
class UAskirHUDWidget;
class AAskirDornfeldEventActor;
class AAskirMerchantActor;
class AAskirCraftingStationActor;
class AAskirDeliveryQuestActor;

UCLASS()
class ASKIR_API AAskirPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    UFUNCTION(BlueprintCallable, Category="Askir|UI") void TogglePauseMenu();
    UFUNCTION(BlueprintCallable, Category="Askir|UI") void ToggleInventory();
    UFUNCTION(BlueprintCallable, Category="Askir|UI") void ToggleJournal();
    UFUNCTION(BlueprintCallable, Category="Askir|UI") void ToggleMagic();
    void CloseAnyMenu();
    UFUNCTION(BlueprintCallable, Category="Askir|UI") void ShowNotification(FText Text);
    bool OpenElderDialogue(AAskirDornfeldEventActor* Elder);
    bool OpenMerchant(AAskirMerchantActor* Merchant);
    bool OpenCraftingStation(AAskirCraftingStationActor* Station);
    bool OpenDeliveryDialogue(AAskirDeliveryQuestActor* Giver);
    void SetMenuInput(bool bOpen, bool bPause);
    bool IsMenuOpen() const {return bMenuOpen;}
    void RefreshInventoryPanel();
private:
    UPROPERTY(Transient) TObjectPtr<UAskirHUDWidget> RuntimeHUD;
    bool bMenuOpen=false;
};
