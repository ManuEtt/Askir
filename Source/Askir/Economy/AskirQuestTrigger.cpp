#include "Economy/AskirQuestTrigger.h"
#include "Quest/AskirQuestComponent.h"
#include "Components/BoxComponent.h"
AAskirQuestTrigger::AAskirQuestTrigger()
{
 PrimaryActorTick.bCanEverTick=false;
 Trigger=CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));RootComponent=Trigger;
 Trigger->SetBoxExtent(FVector(160.f));Trigger->SetCollisionProfileName(TEXT("Trigger"));
 Trigger->OnComponentBeginOverlap.AddDynamic(this,&AAskirQuestTrigger::OnEnter);
}
void AAskirQuestTrigger::OnEnter(UPrimitiveComponent* Overlapped,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
 if(!IsValid(OtherActor)||QuestID.IsNone()||ObjectiveID.IsNone()||Amount<=0)return;
 if(bOneShotPerActor&&TriggeredActors.Contains(OtherActor))return;
 UAskirQuestComponent* Quests=OtherActor->FindComponentByClass<UAskirQuestComponent>();
 if(Quests&&Quests->AdvanceQuestObjective(QuestID,ObjectiveID,Amount)&&bOneShotPerActor)TriggeredActors.Add(OtherActor);
}
