#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/AskirTypes.h"
#include "AskirRelationshipComponent.generated.h"
USTRUCT(BlueprintType) struct FAskirRelationship{GENERATED_BODY() UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) float Affinity=0; UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) float Trust=0; UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) float Fear=0; UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) float Respect=0; UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) float Suspicion=0;};
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent)) class ASKIR_API UAskirRelationshipComponent:public UActorComponent{GENERATED_BODY() public: UAskirRelationshipComponent(); UFUNCTION(BlueprintCallable) void ModifyRelationship(AActor*Other,float A,float T,float F,float R,float S); UFUNCTION(BlueprintPure) FAskirRelationship GetRelationship(AActor*Other)const; UPROPERTY(EditAnywhere,SaveGame) TMap<FAskirEntityID,FAskirRelationship> Relationships;};
