#include "Characters/AskirNPCAdvanced.h"
AAskirNPCAdvanced::AAskirNPCAdvanced(){ScheduleComponent=CreateDefaultSubobject<UAskirScheduleComponent>(TEXT("Schedule"));RelationshipComponent=CreateDefaultSubobject<UAskirRelationshipComponent>(TEXT("Relationships"));FactionComponent=CreateDefaultSubobject<UAskirFactionComponent>(TEXT("Faction"));PerceptionComponent=CreateDefaultSubobject<UAskirPerceptionComponent>(TEXT("Perception"));}
void AAskirNPCAdvanced::Tick(float D){Super::Tick(D);if(!IsDead()&&IsEncounterActive()&&NeedsComponent)NeedsComponent->AdvanceNeeds(D/60.f);}
