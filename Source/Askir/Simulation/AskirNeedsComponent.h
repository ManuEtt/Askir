#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirNeedsComponent.generated.h"
USTRUCT(BlueprintType) struct FAskirNeeds { GENERATED_BODY() UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) float Hunger=0,Thirst=0,Fatigue=0,Stress=0,Safety=1; };
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent)) class ASKIR_API UAskirNeedsComponent:public UActorComponent { GENERATED_BODY() public: UAskirNeedsComponent(); UFUNCTION(BlueprintCallable) void AdvanceNeeds(float Minutes); UFUNCTION(BlueprintCallable) void SatisfyHunger(float A); UFUNCTION(BlueprintCallable) void SatisfyThirst(float A); UFUNCTION(BlueprintCallable) void Rest(float Minutes); UFUNCTION(BlueprintPure) float GetUrgency()const; UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) FAskirNeeds Needs; UPROPERTY(EditAnywhere) float HungerRate=.035f,ThirstRate=.05f,FatigueRate=.025f; };
