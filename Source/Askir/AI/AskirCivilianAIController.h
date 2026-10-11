#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "NPC/AskirDailyRoutineComponent.h"
#include "AskirCivilianAIController.generated.h"

/** NPC travel to level actors with routine destination tags; needs a NavMeshBoundsVolume. */
UCLASS(Blueprintable)
class ASKIR_API AAskirCivilianAIController : public AAIController
{
 GENERATED_BODY()
public:
 AAskirCivilianAIController();
 virtual void OnPossess(APawn* InPawn) override;
 virtual void OnUnPossess() override;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Civilian") float AcceptanceRadius=110.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Civilian") bool bChooseNearestDestination=true;
 UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="Askir|Civilian") FName ActiveDestination=NAME_None;
 UFUNCTION(BlueprintCallable,Category="Askir|Civilian") bool TravelToTag(FName DestinationTag);
 UFUNCTION(BlueprintCallable,Category="Askir|Civilian") bool RefreshDestination();
protected:
 UFUNCTION() void HandleRoutineChanged(EAskirDailyActivity Activity,FName DestinationTag);
private:
 UPROPERTY(Transient) TObjectPtr<UAskirDailyRoutineComponent> Routine;
};
