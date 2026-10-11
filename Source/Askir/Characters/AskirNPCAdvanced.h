#pragma once
#include "CoreMinimal.h"
#include "Characters/AskirNPC.h"
#include "Simulation/AskirNeedsComponent.h"
#include "Simulation/AskirScheduleComponent.h"
#include "Social/AskirRelationshipComponent.h"
#include "Social/AskirFactionComponent.h"
#include "AI/AskirPerceptionComponent.h"
#include "AskirNPCAdvanced.generated.h"
UCLASS()class ASKIR_API AAskirNPCAdvanced:public AAskirNPC{GENERATED_BODY()public:AAskirNPCAdvanced();virtual void Tick(float)override;UPROPERTY(VisibleAnywhere,BlueprintReadOnly)TObjectPtr<UAskirNeedsComponent>NeedsComponent;UPROPERTY(VisibleAnywhere,BlueprintReadOnly)TObjectPtr<UAskirScheduleComponent>ScheduleComponent;UPROPERTY(VisibleAnywhere,BlueprintReadOnly)TObjectPtr<UAskirRelationshipComponent>RelationshipComponent;UPROPERTY(VisibleAnywhere,BlueprintReadOnly)TObjectPtr<UAskirFactionComponent>FactionComponent;UPROPERTY(VisibleAnywhere,BlueprintReadOnly)TObjectPtr<UAskirPerceptionComponent>PerceptionComponent;};
