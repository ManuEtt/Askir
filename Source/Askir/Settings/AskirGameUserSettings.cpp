#include "Settings/AskirGameUserSettings.h"
#include "Settings/AskirSettingsRules.h"
#include "AudioDevice.h"
#include "AudioDeviceManager.h"
#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Sound/AudioSettings.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

namespace
{
struct FAskirSettingsConfigBackup
{
    FString Filename;
    TArray<uint8> Bytes;
    FConfigFile Cached;
    bool bExisted=false;
    bool bHadCache=false;

    bool Capture(const FString& Path)
    {
        if(Path.IsEmpty()||!GConfig)return false;
        Filename=Path;bExisted=IFileManager::Get().FileExists(*Filename);
        if(bExisted&&!FFileHelper::LoadFileToArray(Bytes,*Filename))return false;
        if(const FConfigFile* Existing=GConfig->FindConfigFile(Filename)){Cached=*Existing;bHadCache=true;}
        return true;
    }
    void Restore() const
    {
        bool bRestored=true;
        if(bExisted)
        {
            const FString Temporary=FPaths::CreateTempFilename(*FPaths::GetPath(Filename),TEXT("AskirSettingsRollback"),TEXT(".ini"));
            bRestored=FFileHelper::SaveArrayToFile(Bytes,*Temporary)&&IFileManager::Get().Move(*Filename,*Temporary,true,false);
            IFileManager::Get().Delete(*Temporary,false,false);
        }
        else if(IFileManager::Get().FileExists(*Filename))bRestored=IFileManager::Get().Delete(*Filename,false,false);
        if(GConfig)
        {
            if(bHadCache)
            {
                if(FConfigFile* Existing=GConfig->FindConfigFile(Filename))*Existing=Cached;
            }
            else GConfig->UnloadFile(Filename);
        }
        if(!bRestored)UE_LOG(LogTemp,Error,TEXT("ASKIR: Failed to restore settings file after a failed write: %s"),*Filename);
    }
};
}

UAskirGameUserSettings* UAskirGameUserSettings::Get()
{
    return GEngine?Cast<UAskirGameUserSettings>(GEngine->GetGameUserSettings()):nullptr;
}

bool UAskirGameUserSettings::ValidateDraft(const FAskirSettingsDraft& D)
{
    using namespace AskirSettingsRules;
    return ValidVolume(D.MasterVolume)&&ValidSensitivity(D.CameraSensitivity)&&
        ValidResolution(D.Resolution.X,D.Resolution.Y)&&ValidWindowMode(static_cast<int>(D.WindowMode))&&
        ValidQuality(D.QualityLevel)&&ValidFrameRate(D.FrameRateLimit);
}

FAskirSettingsDraft UAskirGameUserSettings::CaptureDraft() const
{
    FAskirSettingsDraft D;
    D.MasterVolume=GetEffectiveMasterVolume();D.CameraSensitivity=GetEffectiveCameraSensitivity();D.bInvertCameraY=bInvertCameraY;
    D.Resolution=GetScreenResolution();D.WindowMode=static_cast<EAskirWindowMode>(GetFullscreenMode());
    D.QualityLevel=GetOverallScalabilityLevel();D.bVSync=IsVSyncEnabled();D.FrameRateLimit=GetFrameRateLimit();
    return D;
}

bool UAskirGameUserSettings::BeginEdit()
{
    // Reopening the same page must not replace its rollback point with a preview.
    if(Session.IsEditing())return false;
    ValidateSettings();
    OriginalDraft=CaptureDraft();
    if(!ValidateDraft(OriginalDraft))return false;
    OriginalQualityLevels=ScalabilityQuality;
    return Session.Begin();
}

void UAskirGameUserSettings::ApplyNonVideoDraft(const FAskirSettingsDraft& D)
{
    MasterVolume=D.MasterVolume;CameraSensitivity=D.CameraSensitivity;bInvertCameraY=D.bInvertCameraY;
    if(D.QualityLevel>=0)SetOverallScalabilityLevel(D.QualityLevel);
    SetVSyncEnabled(D.bVSync);SetFrameRateLimit(D.FrameRateLimit);
    ApplyNonResolutionSettings();
}

bool UAskirGameUserSettings::ApplyDraft(const FAskirSettingsDraft& D)
{
    if(!Session.CanApply()||!ValidateDraft(D))return false;
    ApplyNonVideoDraft(D);
    return true;
}

bool UAskirGameUserSettings::SaveDraft(const FAskirSettingsDraft& D)
{
    if(!Session.CanApply()||!ValidateDraft(D))return false;
    const FIntPoint Current=GetScreenResolution();
    if(!AskirSettingsRules::CanSaveWithoutPreview(D.Resolution.X,D.Resolution.Y,static_cast<int>(D.WindowMode),
        Current.X,Current.Y,static_cast<int>(GetFullscreenMode())))return false;
    ApplyNonVideoDraft(D);
    ConfirmVideoMode();
    if(!SaveAndVerifySettings())return false;
    return Session.Finish();
}

bool UAskirGameUserSettings::PreviewVideo(const FAskirSettingsDraft& D)
{
    if(!Session.CanApply()||!ValidateDraft(D)||!Session.StartPreview(FPlatformTime::Seconds()))return false;
    ApplyNonVideoDraft(D);
    SetScreenResolution(D.Resolution);SetFullscreenMode(static_cast<EWindowMode::Type>(D.WindowMode));
    // UGameUserSettings::ApplySettings saves to disk, so previews deliberately
    // invoke the two individual apply operations instead.
    ApplyResolutionSettings(false);
    ConfirmationTicker=FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this,[this](float)
    {
        TickVideoConfirmation();return Session.HasPreview();
    }),.1f);
    return true;
}

bool UAskirGameUserSettings::ConfirmVideo()
{
    if(!Session.IsEditing()||!Session.HasPreview())return false;
    const double Now=FPlatformTime::Seconds();
    if(Session.NeedsRollback(Now)||!ValidateDraft(CaptureDraft()))
    {
        RevertVideo();return false;
    }
    ConfirmVideoMode();
    if(!SaveAndVerifySettings())return false;
    StopConfirmationTicker();
    return Session.Confirm(Now);
}

void UAskirGameUserSettings::RestoreOriginal()
{
    if(!Session.IsEditing())return;
    StopConfirmationTicker();
    MasterVolume=OriginalDraft.MasterVolume;CameraSensitivity=OriginalDraft.CameraSensitivity;bInvertCameraY=OriginalDraft.bInvertCameraY;
    ScalabilityQuality=OriginalQualityLevels;
    SetVSyncEnabled(OriginalDraft.bVSync);SetFrameRateLimit(OriginalDraft.FrameRateLimit);
    SetScreenResolution(OriginalDraft.Resolution);SetFullscreenMode(static_cast<EWindowMode::Type>(OriginalDraft.WindowMode));
    ConfirmVideoMode();
    ApplyResolutionSettings(false);ApplyNonResolutionSettings();
    Session.RevertPreview();
}

void UAskirGameUserSettings::RevertVideo()
{
    if(!Session.HasPreview())return;
    RestoreOriginal();
}

void UAskirGameUserSettings::CancelEdit()
{
    RestoreOriginal();Session.Cancel();
}

void UAskirGameUserSettings::TickVideoConfirmation()
{
    if(Session.NeedsRollback(FPlatformTime::Seconds()))RevertVideo();
}

int32 UAskirGameUserSettings::GetVideoSecondsRemaining() const
{
    return Session.Remaining(FPlatformTime::Seconds());
}

float UAskirGameUserSettings::GetEffectiveMasterVolume() const{return static_cast<float>(AskirSettingsRules::EffectiveVolume(MasterVolume));}
float UAskirGameUserSettings::GetEffectiveCameraSensitivity() const{return static_cast<float>(AskirSettingsRules::EffectiveSensitivity(CameraSensitivity));}

void UAskirGameUserSettings::SanitizeCustomSettings()
{
    MasterVolume=GetEffectiveMasterVolume();CameraSensitivity=GetEffectiveCameraSensitivity();
}

void UAskirGameUserSettings::SetToDefaults()
{
    Super::SetToDefaults();MasterVolume=1.f;CameraSensitivity=1.f;bInvertCameraY=false;
    SetScreenResolution(FIntPoint(1280,720));SetFrameRateLimit(60.f);
}

void UAskirGameUserSettings::LoadSettings(bool bForceReload)
{
    // Loading config during an edit must not leave a stale rollback snapshot.
    StopConfirmationTicker();Session.Cancel();
    Super::LoadSettings(bForceReload);SanitizeCustomSettings();
}

void UAskirGameUserSettings::ValidateSettings()
{
    Super::ValidateSettings();SanitizeCustomSettings();
    if(!AskirSettingsRules::ValidResolution(GetScreenResolution().X,GetScreenResolution().Y))
    {
        const FIntPoint Desktop=GetDesktopResolution();
        SetScreenResolution(AskirSettingsRules::ValidResolution(Desktop.X,Desktop.Y)?Desktop:FIntPoint(1280,720));
    }
    if(!AskirSettingsRules::ValidFrameRate(GetFrameRateLimit()))SetFrameRateLimit(60.f);
}

void UAskirGameUserSettings::ApplyAudioVolume()
{
    if(!GEngine)return;
    FAudioDeviceHandle AudioDevice=GEngine->GetMainAudioDevice();
    if(!AudioDevice.IsValid())return;
    const UAudioSettings* AudioSettings=GetDefault<UAudioSettings>();
    USoundClass* Root=AudioSettings?Cast<USoundClass>(AudioSettings->DefaultSoundClassName.TryLoad()):nullptr;
    if(!Root)return;
    // Respect the configured sound-class hierarchy, including custom projects.
    // An invalid cyclic hierarchy must never hang the settings menu.
    TSet<USoundClass*> Visited;
    while(Root->ParentClass)
    {
        if(Visited.Contains(Root))return;
        Visited.Add(Root);Root=Root->ParentClass;
    }
    if(!MasterVolumeMix)
    {
        MasterVolumeMix=NewObject<USoundMix>(this,NAME_None,RF_Transient);
        MasterVolumeMix->Duration=-1.f;MasterVolumeMix->FadeInTime=0.f;MasterVolumeMix->FadeOutTime=0.f;
    }
    // Replace only our modifier, so repeated previews have one active reference
    // and a world/audio flush is repaired by the next settings application.
    AudioDevice->ClearSoundMixModifier(MasterVolumeMix);
    AudioDevice->SetSoundMixClassOverride(MasterVolumeMix,Root,GetEffectiveMasterVolume(),1.f,0.f,true);
    AudioDevice->PushSoundMixModifier(MasterVolumeMix);
}

void UAskirGameUserSettings::ApplyNonResolutionSettings()
{
    Super::ApplyNonResolutionSettings();ApplyAudioVolume();
}

void UAskirGameUserSettings::SaveSettings()
{
    // Engine lifecycle calls must not commit an unconfirmed menu preview.
    // The explicitly accepted path calls the base implementation below and
    // verifies the bytes that were written.
    if(Session.IsEditing()||!ValidateDraft(CaptureDraft()))return;
    Super::SaveSettings();
}

void UAskirGameUserSettings::StopConfirmationTicker()
{
    if(ConfirmationTicker.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(ConfirmationTicker);ConfirmationTicker.Reset();
    }
}

void UAskirGameUserSettings::BeginDestroy()
{
    // Window restoration belongs to CancelEdit before teardown. Never issue a
    // resolution request while the engine itself is destroying user settings.
    StopConfirmationTicker();Session.Cancel();
    Super::BeginDestroy();
}

bool UAskirGameUserSettings::SaveAndVerifySettings()
{
    if(!ValidateDraft(CaptureDraft()))return false;
    const FString QualityFilename=GIsEditor?GEditorSettingsIni:GGameUserSettingsIni;
    FAskirSettingsConfigBackup UserBackup,QualityBackup;
    if(!UserBackup.Capture(GGameUserSettingsIni)||
       (QualityFilename!=GGameUserSettingsIni&&!QualityBackup.Capture(QualityFilename)))return false;
    Super::SaveSettings();
    if(GConfig)
    {
        GConfig->Flush(false,GGameUserSettingsIni);
        if(QualityFilename!=GGameUserSettingsIni)GConfig->Flush(false,QualityFilename);
    }
    // SaveSettings has a void return type. Read a fresh file instead of the
    // in-memory config cache, so a failed write is not reported as success.
    const auto Verify=[this,&QualityFilename]()
    {
    FConfigFile Disk;
    Disk.Read(GGameUserSettingsIni);
    const FString Section=GetClass()->GetPathName();
    float Volume=0.f,Sensitivity=0.f,FrameLimit=0.f;
    bool Invert=false,VSync=false;
    int32 Width=0,Height=0,Mode=-1;
    if(!Disk.GetFloat(*Section,TEXT("MasterVolume"),Volume)||!Disk.GetFloat(*Section,TEXT("CameraSensitivity"),Sensitivity)||
       !Disk.GetBool(*Section,TEXT("bInvertCameraY"),Invert)||!Disk.GetFloat(*Section,TEXT("FrameRateLimit"),FrameLimit)||
       !Disk.GetBool(*Section,TEXT("bUseVSync"),VSync)||!Disk.GetInt(*Section,TEXT("ResolutionSizeX"),Width)||
       !Disk.GetInt(*Section,TEXT("ResolutionSizeY"),Height)||!Disk.GetInt(*Section,TEXT("FullscreenMode"),Mode))return false;
    const FIntPoint Resolution=GetScreenResolution();
    if(!AskirSettingsRules::ValidVolume(Volume)||!AskirSettingsRules::ValidSensitivity(Sensitivity)||!AskirSettingsRules::ValidFrameRate(FrameLimit)||
       !FMath::IsNearlyEqual(Volume,MasterVolume,.0001f)||!FMath::IsNearlyEqual(Sensitivity,CameraSensitivity,.0001f)||
       !FMath::IsNearlyEqual(FrameLimit,GetFrameRateLimit(),.0001f)||Invert!=bInvertCameraY||VSync!=IsVSyncEnabled()||
       Width!=Resolution.X||Height!=Resolution.Y||Mode!=static_cast<int32>(GetFullscreenMode()))return false;
    FConfigFile QualityDisk;
    QualityDisk.Read(QualityFilename);
    const TCHAR* QualityKeys[]={TEXT("sg.ViewDistanceQuality"),TEXT("sg.AntiAliasingQuality"),TEXT("sg.ShadowQuality"),
        TEXT("sg.GlobalIlluminationQuality"),TEXT("sg.ReflectionQuality"),TEXT("sg.PostProcessQuality"),TEXT("sg.TextureQuality"),
        TEXT("sg.EffectsQuality"),TEXT("sg.FoliageQuality"),TEXT("sg.ShadingQuality")};
    const int32 QualityValues[]={ScalabilityQuality.ViewDistanceQuality,ScalabilityQuality.AntiAliasingQuality,ScalabilityQuality.ShadowQuality,
        ScalabilityQuality.GlobalIlluminationQuality,ScalabilityQuality.ReflectionQuality,ScalabilityQuality.PostProcessQuality,ScalabilityQuality.TextureQuality,
        ScalabilityQuality.EffectsQuality,ScalabilityQuality.FoliageQuality,ScalabilityQuality.ShadingQuality};
    for(int32 Index=0;Index<UE_ARRAY_COUNT(QualityKeys);++Index)
    {
        int32 Value=-1;
        if(!QualityDisk.GetInt(TEXT("ScalabilityGroups"),QualityKeys[Index],Value)||Value!=QualityValues[Index])return false;
    }
    float ResolutionQuality=0.f;
    return QualityDisk.GetFloat(TEXT("ScalabilityGroups"),TEXT("sg.ResolutionQuality"),ResolutionQuality)&&
        FMath::IsFinite(ResolutionQuality)&&FMath::IsNearlyEqual(ResolutionQuality,ScalabilityQuality.ResolutionQuality,.01f);
    };
    if(Verify())return true;
    // This is best effort if the filesystem itself is failing. The UI keeps
    // the edit open and reports failure; it never claims successful persistence.
    UserBackup.Restore();
    if(QualityFilename!=GGameUserSettingsIni)QualityBackup.Restore();
    return false;
}
