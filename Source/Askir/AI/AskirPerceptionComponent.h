#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirPerceptionComponent.generated.h"
USTRUCT(BlueprintType) struct FAskirMemory{GENERATED_BODY() UPROPERTY(BlueprintReadOnly) TObjectPtr<AActor> Actor=nullptr; UPROPERTY(BlueprintReadOnly) FVector LastKnownLocation=FVector::ZeroVector; UPROPERTY(BlueprintReadOnly) float Threat=0,Confidence=0; UPROPERTY(BlueprintReadOnly) double LastSeenTime=0;};
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent)) class ASKIR_API UAskirPerceptionComponent:public UActorComponent{GENERATED_BODY() public: UAskirPerceptionComponent(); UFUNCTION(BlueprintCallable) void Remember(AActor*A,float Threat,float Confidence); UFUNCTION(BlueprintPure) TArray<FAskirMemory> GetKnownActors()const{return Memories;} protected: virtual void TickComponent(float,ELevelTick,FActorComponentTickFunction*)override; UPROPERTY() TArray<FAskirMemory> Memories; UPROPERTY(EditAnywhere) float ForgetAfterSeconds=300;};
