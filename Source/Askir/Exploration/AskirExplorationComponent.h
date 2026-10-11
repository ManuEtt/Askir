#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirExplorationComponent.generated.h"

UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirExplorationComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Exploration") int32 DiscoveredSites=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Exploration") int32 LoreKnowledge=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Exploration") float SurveyProgress=0.f;
 UFUNCTION(BlueprintCallable,Category="Askir|Exploration") void DiscoverSite();
 UFUNCTION(BlueprintCallable,Category="Askir|Exploration") void ReadLore();
 UFUNCTION(BlueprintCallable,Category="Askir|Exploration") void Survey(float Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Exploration") void ResetSurvey();
 UFUNCTION(BlueprintCallable,Category="Askir|Exploration") void StudyRuins(int32 Hours);
};
