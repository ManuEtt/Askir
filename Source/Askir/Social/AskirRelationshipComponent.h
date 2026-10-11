#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/AskirTypes.h"
#include "AskirRelationshipComponent.generated.h"
USTRUCT(BlueprintType) struct FAskirRelationship{GENERATED_BODY() UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) float Affinity=0,Trust=0,Fear=0,Respect=0,Suspicion=0;};
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent)) class ASKIR_API UAskirRelationshipComponent:public UActorComponent{GENERATED_BODY() public: UAskirRelationshipComponent(); UFUNCTION(BlueprintCallable) void ModifyRelationship(AActor*Other,float A,float T,float F,float R,float S); UFUNCTION(BlueprintPure) FAskirRelationship GetRelationship(AActor*Other)const; UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) TMap<FAskirEntityID,FAskirRelationship> Relationships;};
