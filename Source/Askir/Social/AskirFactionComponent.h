#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirFactionComponent.generated.h"
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent)) class ASKIR_API UAskirFactionComponent:public UActorComponent{GENERATED_BODY() public: UAskirFactionComponent(); UFUNCTION(BlueprintPure) bool IsHostileTo(FName F)const; UFUNCTION(BlueprintCallable) void SetStanding(FName F,float V); UFUNCTION(BlueprintPure) float GetStanding(FName F)const; UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) FName Faction; UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame) TMap<FName,float> Standings;};
