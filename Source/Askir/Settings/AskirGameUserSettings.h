#pragma once
#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "GameFramework/GameUserSettings.h"
#include "Settings/AskirSettingsRules.h"
#include "AskirGameUserSettings.generated.h"

class USoundMix;

UENUM(BlueprintType)
enum class EAskirWindowMode : uint8
{
    Fullscreen=0,
    WindowedFullscreen=1,
    Windowed=2
};

USTRUCT(BlueprintType)
struct ASKIR_API FAskirSettingsDraft
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere,BlueprintReadWrite) float MasterVolume=1.f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) float CameraSensitivity=1.f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bInvertCameraY=false;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FIntPoint Resolution=FIntPoint(1280,720);
    UPROPERTY(EditAnywhere,BlueprintReadWrite) EAskirWindowMode WindowMode=EAskirWindowMode::WindowedFullscreen;
    // -1 preserves individually configured scalability settings.
    UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 QualityLevel=3;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bVSync=false;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) float FrameRateLimit=60.f;
};

/** Preview changes in memory; write GameUserSettings.ini only on explicit acceptance. */
UCLASS(Config=GameUserSettings)
class ASKIR_API UAskirGameUserSettings : public UGameUserSettings
{
    GENERATED_BODY()
public:
    static UAskirGameUserSettings* Get();
    static bool ValidateDraft(const FAskirSettingsDraft& Draft);

    UFUNCTION(BlueprintPure,Category="Askir|Settings") FAskirSettingsDraft CaptureDraft() const;
    UFUNCTION(BlueprintCallable,Category="Askir|Settings") bool BeginEdit();
    UFUNCTION(BlueprintCallable,Category="Askir|Settings") bool ApplyDraft(const FAskirSettingsDraft& Draft);
    UFUNCTION(BlueprintCallable,Category="Askir|Settings") bool SaveDraft(const FAskirSettingsDraft& Draft);
    UFUNCTION(BlueprintCallable,Category="Askir|Settings") bool PreviewVideo(const FAskirSettingsDraft& Draft);
    UFUNCTION(BlueprintCallable,Category="Askir|Settings") bool ConfirmVideo();
    UFUNCTION(BlueprintCallable,Category="Askir|Settings") void RevertVideo();
    UFUNCTION(BlueprintCallable,Category="Askir|Settings") void CancelEdit();
    UFUNCTION(BlueprintCallable,Category="Askir|Settings") void TickVideoConfirmation();
    UFUNCTION(BlueprintPure,Category="Askir|Settings") bool IsEditing() const{return Session.IsEditing();}
    UFUNCTION(BlueprintPure,Category="Askir|Settings") bool HasVideoConfirmation() const{return Session.HasPreview();}
    UFUNCTION(BlueprintPure,Category="Askir|Settings") int32 GetVideoSecondsRemaining() const;
    UFUNCTION(BlueprintPure,Category="Askir|Settings") float GetEffectiveMasterVolume() const;
    UFUNCTION(BlueprintPure,Category="Askir|Settings") float GetEffectiveCameraSensitivity() const;
    UFUNCTION(BlueprintPure,Category="Askir|Settings") bool GetEffectiveInvertCameraY() const{return bInvertCameraY;}

    virtual void SetToDefaults() override;
    virtual void LoadSettings(bool bForceReload=false) override;
    virtual void ValidateSettings() override;
    virtual void ApplyNonResolutionSettings() override;
    virtual void SaveSettings() override;
    virtual void BeginDestroy() override;

private:
    UPROPERTY(Config) float MasterVolume=1.f;
    UPROPERTY(Config) float CameraSensitivity=1.f;
    UPROPERTY(Config) bool bInvertCameraY=false;
    UPROPERTY(Transient) FAskirSettingsDraft OriginalDraft;
    UPROPERTY(Transient) TObjectPtr<USoundMix> MasterVolumeMix;
    Scalability::FQualityLevels OriginalQualityLevels;
    AskirSettingsRules::EditSession Session;
    FTSTicker::FDelegateHandle ConfirmationTicker;

    void ApplyNonVideoDraft(const FAskirSettingsDraft& Draft);
    void RestoreOriginal();
    void ApplyAudioVolume();
    void SanitizeCustomSettings();
    void StopConfirmationTicker();
    bool SaveAndVerifySettings();
};
