#include "Mission/AskirDeliveryQuestActor.h"
#include "Components/SceneComponent.h"
#include "Mission/AskirDeliveryMissionComponent.h"
#include "Characters/AskirCharacter.h"
#include "Game/AskirPlayerController.h"
#include "Game/AskirGameMode.h"
#include "Quest/AskirQuestComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AAskirDeliveryQuestActor::AAskirDeliveryQuestActor()
{
    PrimaryActorTick.bCanEverTick=false;
    SceneRoot=CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));SetRootComponent(SceneRoot);
    Visual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));Visual->SetupAttachment(SceneRoot);
    Visual->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    DisplayName=NSLOCTEXT("AskirDelivery","TorbenName","Torben");
}
void AAskirDeliveryQuestActor::BeginPlay()
{
    Super::BeginPlay();
    if(!BindMissionEvents() && GetWorld())GetWorld()->GetTimerManager().SetTimer(BindRetryTimer,this,&AAskirDeliveryQuestActor::RefreshMissionState,.5f,true);
}
bool AAskirDeliveryQuestActor::BindMissionEvents()
{
    if(!IsValid(MissionPlayer))MissionPlayer=UGameplayStatics::GetPlayerPawn(this,0);
    if(!IsValid(MissionPlayer))return false;
    UAskirQuestComponent* Journal=MissionPlayer->FindComponentByClass<UAskirQuestComponent>();
    if(!Journal)return false;
    if(BoundJournal!=Journal)
    {
        if(BoundJournal)BoundJournal->OnQuestProgressChanged.RemoveDynamic(this,&AAskirDeliveryQuestActor::OnQuestProgressChanged);
        BoundJournal=Journal;
        BoundJournal->OnQuestProgressChanged.AddUniqueDynamic(this,&AAskirDeliveryQuestActor::OnQuestProgressChanged);
    }
    if(GetWorld())GetWorld()->GetTimerManager().ClearTimer(BindRetryTimer);
    RefreshMissionState();return true;
}
void AAskirDeliveryQuestActor::RefreshMissionState()
{
    if(!BoundJournal){BindMissionEvents();return;}
    if(Role!=EAskirDeliveryQuestRole::Cargo)return;
    const UAskirDeliveryMissionComponent* Mission=MissionPlayer?MissionPlayer->FindComponentByClass<UAskirDeliveryMissionComponent>():nullptr;
    const bool bRecovered=Mission && Mission->HasRecoveredCargo(EventID);
    SetActorHiddenInGame(bRecovered);SetActorEnableCollision(!bRecovered);
}
void AAskirDeliveryQuestActor::OnQuestProgressChanged(FName QuestID,FName ObjectiveID)
{if(QuestID.IsNone() || QuestID==UAskirDeliveryMissionComponent::GetQuestID())RefreshMissionState();}
bool AAskirDeliveryQuestActor::CanInteractWith(AActor* Interactor) const
{
    const AAskirCharacter* Player=Cast<AAskirCharacter>(Interactor);
    if(!IsValid(Player) || !Player->IsPlayerControlled() || Player->IsDead() || Player->IsWorldTravelPending() || !GetWorld()
        || !FMath::IsFinite(InteractionRange) || InteractionRange<=0.f
        || Player->GetActorLocation().ContainsNaN() || GetActorLocation().ContainsNaN()
        || FVector::DistSquared(Player->GetActorLocation(),GetActorLocation())>double(InteractionRange)*double(InteractionRange))return false;
    const UAskirDeliveryMissionComponent* Mission=Player->FindComponentByClass<UAskirDeliveryMissionComponent>();
    if(!Mission || Mission->IsTransactionInProgress())return false;
    if(const auto* Mode=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))if(Mode->IsPersistenceBusy())return false;
    return Role==EAskirDeliveryQuestRole::Giver || (Role==EAskirDeliveryQuestRole::Cargo && !EventID.IsNone() && !Mission->HasRecoveredCargo(EventID));
}
bool AAskirDeliveryQuestActor::Interact_Implementation(AActor* Interactor)
{
    if(!CanInteractWith(Interactor))return false;
    AAskirCharacter* Player=Cast<AAskirCharacter>(Interactor);
    if(Role==EAskirDeliveryQuestRole::Giver)
    {
        AAskirPlayerController* Controller=Cast<AAskirPlayerController>(Player->GetController());
        return Controller && Controller->OpenDeliveryDialogue(this);
    }
    UAskirDeliveryMissionComponent* Mission=Player->FindComponentByClass<UAskirDeliveryMissionComponent>();
    const bool bChanged=Mission && Mission->RecoverCargo(EventID);
    if(bChanged)
    {
        MissionPlayer=Player;
        BindMissionEvents();RefreshMissionState();
    }
    return bChanged;
}
FText AAskirDeliveryQuestActor::GetInteractionText_Implementation() const
{
    return Role==EAskirDeliveryQuestRole::Giver
        ?NSLOCTEXT("AskirDelivery","Speak","Mit Torben sprechen")
        :NSLOCTEXT("AskirDelivery","CargoInteract","Torbens Lieferung sichern");
}
void AAskirDeliveryQuestActor::EndPlay(const EEndPlayReason::Type Reason)
{
    if(BoundJournal)BoundJournal->OnQuestProgressChanged.RemoveDynamic(this,&AAskirDeliveryQuestActor::OnQuestProgressChanged);
    if(GetWorld())GetWorld()->GetTimerManager().ClearTimer(BindRetryTimer);
    Super::EndPlay(Reason);
}
