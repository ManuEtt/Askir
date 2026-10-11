#include "Quest/AskirQuestGiver.h"
#include "Quest/AskirQuestComponent.h"
#include "Quest/AskirQuestData.h"
#include "Components/StaticMeshComponent.h"
AAskirQuestGiver::AAskirQuestGiver()
{
 PrimaryActorTick.bCanEverTick=false;
 Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));RootComponent=Mesh;
}
bool AAskirQuestGiver::Interact_Implementation(AActor* Interactor)
{
 if(!IsValid(Interactor)||!IsValid(Quest))return false;
 UAskirQuestComponent* Quests=Interactor->FindComponentByClass<UAskirQuestComponent>();if(!Quests)return false;
 const EAskirQuestState State=Quests->GetState(Quest);
 if(State==EAskirQuestState::Inactive||State==EAskirQuestState::Available)return Quests->StartQuest(Quest);
 if(State==EAskirQuestState::Active&&bAllowTurnIn)return Quests->CompleteQuest(Quest);
 return false;
}
FText AAskirQuestGiver::GetInteractionText_Implementation()const
{
 return IsValid(Quest)?FText::Format(NSLOCTEXT("Askir","QuestGiverPrompt","Quest: {0}"),Quest->Title):NSLOCTEXT("Askir","QuestGiverEmpty","Keine Quest");
}
