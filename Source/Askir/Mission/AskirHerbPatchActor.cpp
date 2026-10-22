#include "Mission/AskirHerbPatchActor.h"
#include "Mission/AskirHerbMissionComponent.h"
#include "Characters/AskirCharacter.h"
#include "Game/AskirGameMode.h"
#include "Game/AskirPlayerController.h"
#include "Quest/AskirQuestComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AAskirHerbPatchActor::AAskirHerbPatchActor()
{
    PrimaryActorTick.bCanEverTick=false;
    SceneRoot=CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));SetRootComponent(SceneRoot);
    Visual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));Visual->SetupAttachment(SceneRoot);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);Visual->SetCanEverAffectNavigation(false);
    InteractionQuery=CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionQuery"));InteractionQuery->SetupAttachment(SceneRoot);
    InteractionQuery->SetBoxExtent(FVector(65.,65.,45.));InteractionQuery->SetRelativeLocation(FVector(0.,0.,-60.));
    InteractionQuery->SetCollisionEnabled(ECollisionEnabled::QueryOnly);InteractionQuery->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionQuery->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);InteractionQuery->SetGenerateOverlapEvents(false);
    InteractionQuery->SetCanEverAffectNavigation(false);
}
void AAskirHerbPatchActor::BeginPlay()
{
    Super::BeginPlay();
    if(!BindMissionEvents()&&!bMissionBindingEnded&&!IsActorBeingDestroyed()&&GetWorld()&&!GetWorld()->bIsTearingDown)
        GetWorld()->GetTimerManager().SetTimer(BindRetryTimer,this,&AAskirHerbPatchActor::RefreshMissionState,.5f,true);
}
bool AAskirHerbPatchActor::BindMissionEvents()
{
    if(bMissionBindingEnded||IsActorBeingDestroyed()||!GetWorld()||GetWorld()->bIsTearingDown)return false;
    if(!IsValid(MissionPlayer)||MissionPlayer->IsActorBeingDestroyed())MissionPlayer=UGameplayStatics::GetPlayerPawn(this,0);
    // GameplayStatics may still expose a pawn during its Destroy/EndPlay call.
    // Never enter RefreshMissionState recursively with that same dying pawn.
    if(!IsValid(MissionPlayer)||MissionPlayer->IsActorBeingDestroyed()||MissionPlayer->GetWorld()!=GetWorld())
    {ClearMissionBinding();return false;}
    auto* Journal=MissionPlayer->FindComponentByClass<UAskirQuestComponent>();if(!Journal)return false;
    if(BoundJournal!=Journal)
    {
        if(BoundJournal)BoundJournal->OnQuestProgressChanged.RemoveDynamic(this,&AAskirHerbPatchActor::OnQuestProgressChanged);
        BoundJournal=Journal;BoundJournal->OnQuestProgressChanged.AddUniqueDynamic(this,&AAskirHerbPatchActor::OnQuestProgressChanged);
    }
    if(GetWorld())GetWorld()->GetTimerManager().ClearTimer(BindRetryTimer);
    RefreshMissionState();return true;
}
void AAskirHerbPatchActor::RefreshMissionState()
{
    if(bMissionBindingEnded||IsActorBeingDestroyed()||!GetWorld()||GetWorld()->bIsTearingDown)return;
    if(!IsValid(BoundJournal)||!IsValid(MissionPlayer)||MissionPlayer->IsActorBeingDestroyed()){BindMissionEvents();return;}
    const auto* Mission=MissionPlayer->FindComponentByClass<UAskirHerbMissionComponent>();
    const bool bExplored=Mission&&Mission->HasExploredHerbs(SourceID);
    SetActorHiddenInGame(bExplored);SetActorEnableCollision(!bExplored);
}
void AAskirHerbPatchActor::OnQuestProgressChanged(FName QuestID,FName ObjectiveID)
{if(QuestID.IsNone()||QuestID==UAskirHerbMissionComponent::GetQuestID())RefreshMissionState();}
bool AAskirHerbPatchActor::CanInteractWith(AActor* Interactor) const
{
    const auto* Player=Cast<AAskirCharacter>(Interactor);
    if(!IsValid(Player)||Player->IsActorBeingDestroyed()||!Player->IsPlayerControlled()||!Player->CanUseGameplayInput()||
        IsActorBeingDestroyed()||IsHidden()||!GetWorld()||GetWorld()->bIsTearingDown||Player->GetWorld()!=GetWorld()||
        SourceID!=UAskirHerbMissionComponent::GetCanonicalSourceID()||!FMath::IsFinite(InteractionRange)||InteractionRange<=0.f||
        Player->GetActorLocation().ContainsNaN()||GetActorLocation().ContainsNaN()||
        FVector::DistSquared(Player->GetActorLocation(),GetActorLocation())>double(InteractionRange)*double(InteractionRange))return false;
    const auto* Mission=Player->FindComponentByClass<UAskirHerbMissionComponent>();
    if(!Mission||Mission->IsTransactionInProgress()||Mission->GetStage()!=EAskirHerbStage::ExploreMeadow||Mission->HasExploredHerbs(SourceID))return false;
    if(const auto* Mode=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))if(Mode->IsPersistenceBusy())return false;
    FVector Eye;FRotator Rotation;Player->GetActorEyesViewPoint(Eye,Rotation);
    const FVector Point=InteractionQuery?InteractionQuery->GetComponentLocation():GetActorLocation();
    if(Eye.ContainsNaN()||Point.ContainsNaN())return false;
    FHitResult Hit;FCollisionQueryParams Query(SCENE_QUERY_STAT(AskirHerbInteractionSight),false,Player);
    return !GetWorld()->LineTraceSingleByChannel(Hit,Eye,Point,ECC_Visibility,Query)||Hit.GetActor()==this;
}
bool AAskirHerbPatchActor::Interact_Implementation(AActor* Interactor)
{
    if(!CanInteractWith(Interactor))return false;
    auto* Mission=Interactor->FindComponentByClass<UAskirHerbMissionComponent>();
    if(!Mission||!Mission->ExploreHerbs(SourceID))return false;
    // The committed exploration callback may destroy the patch or the player,
    // or tear down the world. Preserve its receipt without reviving bindings.
    if(bMissionBindingEnded||IsActorBeingDestroyed()||!IsValid(Interactor)||Interactor->IsActorBeingDestroyed()||
        !GetWorld()||GetWorld()->bIsTearingDown||Interactor->GetWorld()!=GetWorld())
    {ClearMissionBinding();return true;}
    MissionPlayer=Interactor;BindMissionEvents();RefreshMissionState();
    if(const auto* Player=Cast<AAskirCharacter>(Interactor))
        if(auto* Controller=Cast<AAskirPlayerController>(Player->GetController()))
            Controller->ShowNotification(NSLOCTEXT("AskirHerbs","HealthyPatch","Gesunde Heilkräuter gefunden. Berichte Rurik in Dornfeld."));
    return true;
}
FText AAskirHerbPatchActor::GetInteractionText_Implementation() const
{return NSLOCTEXT("AskirHerbs","Inspect","Heilkräutervorkommen untersuchen");}
void AAskirHerbPatchActor::ClearMissionBinding()
{
    // Strong UObject references remain allocated during synchronous EndPlay;
    // remove the subscription even when the journal is already pending destroy.
    if(BoundJournal)BoundJournal->OnQuestProgressChanged.RemoveDynamic(this,&AAskirHerbPatchActor::OnQuestProgressChanged);
    BoundJournal=nullptr;MissionPlayer=nullptr;
}
void AAskirHerbPatchActor::EndPlay(const EEndPlayReason::Type Reason)
{
    bMissionBindingEnded=true;ClearMissionBinding();
    if(GetWorld())GetWorld()->GetTimerManager().ClearTimer(BindRetryTimer);
    Super::EndPlay(Reason);
}
