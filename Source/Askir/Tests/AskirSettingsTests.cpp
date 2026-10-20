#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Settings/AskirGameUserSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirSettingsDraftValidationTest,"Askir.Settings.DraftValidation",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirSettingsDraftValidationTest::RunTest(const FString& Parameters)
{
    FAskirSettingsDraft Draft;
    TestTrue(TEXT("Default draft is valid"),UAskirGameUserSettings::ValidateDraft(Draft));
    Draft.MasterVolume=std::numeric_limits<float>::quiet_NaN();
    TestFalse(TEXT("NaN volume cannot enter an edit transaction"),UAskirGameUserSettings::ValidateDraft(Draft));
    Draft.MasterVolume=0.f;Draft.CameraSensitivity=.2f;
    TestTrue(TEXT("Muted audio and minimum usable sensitivity accepted"),UAskirGameUserSettings::ValidateDraft(Draft));
    Draft.WindowMode=static_cast<EAskirWindowMode>(255);
    TestFalse(TEXT("Unknown window mode rejected"),UAskirGameUserSettings::ValidateDraft(Draft));
    Draft.WindowMode=EAskirWindowMode::Windowed;Draft.Resolution=FIntPoint(0,0);
    TestFalse(TEXT("Zero resolution rejected before apply"),UAskirGameUserSettings::ValidateDraft(Draft));
    Draft.Resolution=FIntPoint(1920,1080);Draft.QualityLevel=-1;
    TestTrue(TEXT("Individually configured quality can be preserved"),UAskirGameUserSettings::ValidateDraft(Draft));
    Draft.FrameRateLimit=std::numeric_limits<float>::infinity();
    TestFalse(TEXT("Infinite frame limit rejected"),UAskirGameUserSettings::ValidateDraft(Draft));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirSettingsTransactionTest,"Askir.Settings.PreviewRollbackWithoutDiskWrites",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirSettingsTransactionTest::RunTest(const FString& Parameters)
{
    // Invalid requests and unopened transactions must not call graphics apply
    // or touch the real user settings file. Engine integration of actual video
    // previews belongs to the separate windowed manual test.
    auto* Settings=NewObject<UAskirGameUserSettings>();
    Settings->SetToDefaults();
    const FAskirSettingsDraft Original=Settings->CaptureDraft();
    FAskirSettingsDraft Changed=Original;Changed.MasterVolume=.25f;Changed.CameraSensitivity=2.f;
    TestFalse(TEXT("No implicit edit session"),Settings->ApplyDraft(Changed));
    TestFalse(TEXT("No implicit disk commit"),Settings->SaveDraft(Changed));
    TestFalse(TEXT("No implicit display change"),Settings->PreviewVideo(Changed));
    TestFalse(TEXT("No confirmation without preview"),Settings->ConfirmVideo());
    TestEqual(TEXT("Rejected draft preserves volume"),Settings->GetEffectiveMasterVolume(),Original.MasterVolume);
    TestEqual(TEXT("Rejected draft preserves sensitivity"),Settings->GetEffectiveCameraSensitivity(),Original.CameraSensitivity);
    Settings->CancelEdit();Settings->RevertVideo();Settings->TickVideoConfirmation();
    TestFalse(TEXT("Repeated cancellation stays closed"),Settings->IsEditing());
    TestEqual(TEXT("No pending preview countdown"),Settings->GetVideoSecondsRemaining(),0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirLiveSettingsRollbackTest,"Askir.Settings.LiveNonVideoTransactionRollback",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirLiveSettingsRollbackTest::RunTest(const FString& Parameters)
{
    UAskirGameUserSettings* Settings=UAskirGameUserSettings::Get();
    if(!TestNotNull(TEXT("Configured native settings subclass"),Settings))return false;
    if(!TestFalse(TEXT("Automation begins outside an interactive settings session"),Settings->IsEditing()))return false;
    const FAskirSettingsDraft Original=Settings->CaptureDraft();
    if(!TestTrue(TEXT("Begin records complete live rollback snapshot"),Settings->BeginEdit()))return false;
    FAskirSettingsDraft Changed=Original;
    Changed.MasterVolume=Original.MasterVolume>.5f?.25f:.75f;
    Changed.CameraSensitivity=Original.CameraSensitivity>1.f?.75f:2.f;
    Changed.bInvertCameraY=!Original.bInvertCameraY;
    TestTrue(TEXT("Live nonvideo preview applies"),Settings->ApplyDraft(Changed));
    TestEqual(TEXT("Audio scalar changed"),Settings->GetEffectiveMasterVolume(),Changed.MasterVolume);
    TestEqual(TEXT("Camera scalar changed"),Settings->GetEffectiveCameraSensitivity(),Changed.CameraSensitivity);
    TestEqual(TEXT("Inversion changed"),Settings->GetEffectiveInvertCameraY(),Changed.bInvertCameraY);
    TestEqual(TEXT("Nonvideo preview preserves display resolution"),Settings->GetScreenResolution(),Original.Resolution);
    TestFalse(TEXT("Reopening does not replace original snapshot"),Settings->BeginEdit());
    FString BeforeFile,AfterFile;
    const bool bHadFile=FFileHelper::LoadFileToString(BeforeFile,*GGameUserSettingsIni);
    Settings->SaveSettings();
    const bool bHasFile=FFileHelper::LoadFileToString(AfterFile,*GGameUserSettingsIni);
    TestEqual(TEXT("External save cannot create or remove an INI during a preview"),bHasFile,bHadFile);
    TestEqual(TEXT("External save cannot persist unaccepted preview bytes"),AfterFile,BeforeFile);
    FAskirSettingsDraft Invalid=Changed;Invalid.CameraSensitivity=std::numeric_limits<float>::infinity();
    TestFalse(TEXT("Invalid draft cannot mutate partially"),Settings->ApplyDraft(Invalid));
    TestEqual(TEXT("Invalid input preserved prior valid camera setting"),Settings->GetEffectiveCameraSensitivity(),Changed.CameraSensitivity);
    Settings->CancelEdit();
    const FAskirSettingsDraft Restored=Settings->CaptureDraft();
    TestFalse(TEXT("Cancel closes session"),Settings->IsEditing());
    TestEqual(TEXT("Cancel restored original audio"),Restored.MasterVolume,Original.MasterVolume);
    TestEqual(TEXT("Cancel restored original sensitivity"),Restored.CameraSensitivity,Original.CameraSensitivity);
    TestEqual(TEXT("Cancel restored original inversion"),Restored.bInvertCameraY,Original.bInvertCameraY);
    TestEqual(TEXT("Cancel restored quality"),Restored.QualityLevel,Original.QualityLevel);
    TestEqual(TEXT("Cancel restored frame cap"),Restored.FrameRateLimit,Original.FrameRateLimit);
    TestEqual(TEXT("Cancel restored VSync"),Restored.bVSync,Original.bVSync);
    return true;
}
#endif
