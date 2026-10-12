#include "World/AskirEncounterZone.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Quest/AskirQuestComponent.h"
AAskirEncounterZone::AAskirEncounterZone()
{
 PrimaryActorTick.bCanEverTick=false;
 Trigger=CreateDefaultSubobject<UBoxComponent>(TEXT("EncounterTrigger"));
 SetRootComponent(Trigger);
 Trigger->SetBoxExtent(FVector(250.f,250.f,150.f));
 Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
 Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
 Trigger->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
 Trigger->SetGenerateOverlapEvents(true);
 Trigger->OnComponentBeginOverlap.AddDynamic(this,&AAskirEncounterZone::HandleOverlap);
}
void AAskirEncounterZone::HandleOverlap(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,int32,bool,const FHitResult&)
{
 ActivateForPlayer(Other);
}
bool AAskirEncounterZone::ActivateForPlayer(AActor* Player)
{
 APawn* Pawn=Cast<APawn>(Player);
 if(!IsValid(Pawn)||!Pawn->IsPlayerControlled()||EncounterID.IsNone())return false;
 if(bOncePerPlayer&&ActivatedPlayers.Contains(Player))return false;
 // Persistent one-shot encounters stay inactive after loading a completed save.
 if(const UAskirQuestComponent* SavedQuests=Pawn->FindComponentByClass<UAskirQuestComponent>())
  if(SavedQuests->HasCompletedEncounter(EncounterID))return false;
 UAskirQuestComponent* Quests=Pawn->FindComponentByClass<UAskirQuestComponent>();
 if(bRequireActiveQuest)
 {
  if(!Quests||QuestID.IsNone()||Quests->GetStateByID(QuestID)!=EAskirQuestState::Active)return false;
 }
 if(bAwardObjectiveOnEntry&&!QuestID.IsNone()&&!ObjectiveID.IsNone()&&Quests&&ObjectiveAmount>0)
 {
  Quests->AdvanceQuestObjective(QuestID,ObjectiveID,ObjectiveAmount);
 }
 if(bOncePerPlayer)ActivatedPlayers.Add(Player);
 OnEncounterTriggered.Broadcast(Player,EncounterID);
 return true;
}
void AAskirEncounterZone::ResetEncounter(){ActivatedPlayers.Reset();}
