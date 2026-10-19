#include "Mission/AskirDeliveryBanditRelay.h"
#include "Mission/AskirDeliveryMissionComponent.h"
#include "Characters/AskirNPC.h"
#include "Combat/AskirCombatComponent.h"
#include "Quest/AskirQuestComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AAskirDeliveryBanditRelay::AAskirDeliveryBanditRelay(){PrimaryActorTick.bCanEverTick=false;}
void AAskirDeliveryBanditRelay::BeginPlay()
{
    Super::BeginPlay();
    const float RetrySeconds=FMath::IsFinite(BindRetryInterval)?FMath::Clamp(BindRetryInterval,.1f,10.f):.5f;
    if(!BindMissionEvents() && GetWorld())GetWorld()->GetTimerManager().SetTimer(BindRetryTimer,this,&AAskirDeliveryBanditRelay::RetryBind,RetrySeconds,true);
}
void AAskirDeliveryBanditRelay::RetryBind()
{if(BindMissionEvents() && GetWorld())GetWorld()->GetTimerManager().ClearTimer(BindRetryTimer);}
bool AAskirDeliveryBanditRelay::BindMissionEvents()
{
    if(bBound)return true;
    if(EventID.IsNone() || !IsValid(BanditActor))return false;
    if(!IsValid(MissionPlayer))MissionPlayer=UGameplayStatics::GetPlayerPawn(this,0);
    if(!IsValid(MissionPlayer))return false;
    BoundMission=MissionPlayer->FindComponentByClass<UAskirDeliveryMissionComponent>();
    BoundJournal=MissionPlayer->FindComponentByClass<UAskirQuestComponent>();
    BanditCombat=BanditActor->FindComponentByClass<UAskirCombatComponent>();
    if(!BoundMission || !BoundJournal || !BanditCombat)return false;
    if(AAskirNPC* NPC=Cast<AAskirNPC>(BanditActor))
    {
        // No reward delegate may pay a separate per-kill reward before this
        // encounter receipt exists. SQ_001 pays once when Torben receives cargo.
        NPC->KillExperience=0;NPC->GuaranteedDrop=nullptr;NPC->DefeatObjectiveID=NAME_None;
    }
    BanditCombat->OnDeath.AddUniqueDynamic(this,&AAskirDeliveryBanditRelay::OnBanditDied);
    BoundJournal->OnQuestProgressChanged.AddUniqueDynamic(this,&AAskirDeliveryBanditRelay::OnQuestProgressChanged);
    bBound=true;RefreshMissionState();return true;
}
void AAskirDeliveryBanditRelay::RefreshMissionState()
{
    if(!bBound){BindMissionEvents();return;}
    ReconcileState(true);
}
void AAskirDeliveryBanditRelay::ReconcileState(bool bForceRestore)
{
    if(!BoundMission || !BanditCombat || !IsValid(BanditActor))return;
    const bool bDefeated=BoundMission->HasCreditedBandit(EventID);
    const bool bActive=!bDefeated && BoundMission->GetStage()==EAskirDeliveryStage::DefeatBandits;
    const bool bChanged=!bInitialized || bDefeated!=bLastDefeated || bActive!=bLastActive;
    if(!bForceRestore && !bChanged)return;
    bInitialized=true;bLastDefeated=bDefeated;bLastActive=bActive;
    BanditActor->SetLifeSpan(0.f);
    if(AAskirNPC* NPC=Cast<AAskirNPC>(BanditActor))
    {
        NPC->RestoreEncounterState(bDefeated);
        NPC->bHostile=bActive;NPC->SetEncounterActive(bActive);
    }
    else
    {
        BanditCombat->RestoreHealth(bDefeated?0.f:BanditCombat->GetMaxHealth());
        BanditCombat->SetDamageEnabled(bActive);
        BanditActor->SetActorHiddenInGame(!bActive);
        BanditActor->SetActorEnableCollision(bActive);
        BanditActor->SetActorTickEnabled(bActive);
    }
}
void AAskirDeliveryBanditRelay::OnQuestProgressChanged(FName QuestID,FName ObjectiveID)
{
    if(QuestID.IsNone() || QuestID==UAskirDeliveryMissionComponent::GetQuestID())ReconcileState(QuestID.IsNone() || ObjectiveID.IsNone());
}
void AAskirDeliveryBanditRelay::OnBanditDied()
{
    if(!BoundMission || !BanditCombat || !IsValid(BanditActor))return;
    BanditActor->SetLifeSpan(0.f);
    const APawn* Player=Cast<APawn>(MissionPlayer);
    if(Player && Player->IsPlayerControlled() && BanditCombat->IsKillAttributedTo(MissionPlayer) && BoundMission->CreditBandit(EventID))return;
    // Environmental/NPC deaths cannot silently consume a required mission actor.
    if(GetWorld())GetWorld()->GetTimerManager().SetTimerForNextTick(this,&AAskirDeliveryBanditRelay::RefreshMissionState);
}
void AAskirDeliveryBanditRelay::EndPlay(const EEndPlayReason::Type Reason)
{
    if(BanditCombat)BanditCombat->OnDeath.RemoveDynamic(this,&AAskirDeliveryBanditRelay::OnBanditDied);
    if(BoundJournal)BoundJournal->OnQuestProgressChanged.RemoveDynamic(this,&AAskirDeliveryBanditRelay::OnQuestProgressChanged);
    if(GetWorld())GetWorld()->GetTimerManager().ClearTimer(BindRetryTimer);
    Super::EndPlay(Reason);
}
