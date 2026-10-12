#include "Audio/AskirAudioLibrary.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "GameFramework/Actor.h"

namespace
{
USoundBase* ResolveCue(FName Cue)
{
    static TMap<FName,TWeakObjectPtr<USoundBase>> Cache;
    if(const auto* Cached=Cache.Find(Cue))if(Cached->IsValid())return Cached->Get();
    const FName Allowed[]={TEXT("Shard"),TEXT("Chest"),TEXT("QuestAccepted"),TEXT("QuestCompleted"),TEXT("MeleeSwing"),TEXT("MeleeHit"),TEXT("DungeonAmbient"),TEXT("SpellCast"),TEXT("SpellImpact")};
    bool bAllowed=false;for(const FName Name:Allowed)if(Cue==Name){bAllowed=true;break;}
    if(!bAllowed)return nullptr;
    const bool bMagic=Cue==TEXT("SpellCast")||Cue==TEXT("SpellImpact");
    const FString Package=(bMagic?TEXT("/Game/Askir/V78/Audio/S_"):TEXT("/Game/Askir/V77/Audio/S_"))+Cue.ToString();
    // Source-only and automation worlds deliberately have no imported packages.
    if(!FPackageName::DoesPackageExist(Package))return nullptr;
    USoundBase* Sound=LoadObject<USoundBase>(nullptr,*(Package+TEXT(".S_")+Cue.ToString()));
    if(Sound)Cache.Add(Cue,Sound);
    return Sound;
}
}
void UAskirAudioLibrary::PlayCue(UObject* WorldContext,FName Cue)
{
    if(IsValid(WorldContext))if(USoundBase* Sound=ResolveCue(Cue))UGameplayStatics::PlaySound2D(WorldContext,Sound);
}
void UAskirAudioLibrary::PlayWorldCue(AActor* Source,FName Cue)
{
    if(IsValid(Source))if(USoundBase* Sound=ResolveCue(Cue))UGameplayStatics::PlaySoundAtLocation(Source,Sound,Source->GetActorLocation());
}
