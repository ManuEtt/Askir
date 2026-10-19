#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AskirInteractable.h"
#include "AskirWorldPortal.generated.h"
class UBoxComponent;
/** A portal only accepts players and opens its configured, cooked map. */
UCLASS(Blueprintable)
class ASKIR_API AAskirWorldPortal : public AActor, public IAskirInteractable
{
 GENERATED_BODY()
public:
 AAskirWorldPortal();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|World") TObjectPtr<UBoxComponent> Trigger;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") FName DestinationMap=NAME_None;
 /** Optional canonical package path; blank keeps legacy WorldV42 maps working. */
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") FString DestinationPackagePath;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") FName PortalID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") FName DestinationPortalID=NAME_None;
 /** Local-space arrival offset, authored outside this portal's trigger. */
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") FVector ArrivalOffset=FVector(0.f,420.f,40.f);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") bool bEnabled=true;
 /** Existing authored worlds keep overlap travel; V80 routes use explicit E interaction. */
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") bool bTravelOnOverlap=true;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World",meta=(ClampMin="1")) float InteractionRange=280.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") FText InteractionLabel;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") FName RequiredQuestID=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") bool bRequireQuestCompleted=false;
 UFUNCTION(BlueprintPure,Category="Askir|World") bool CanTravel(AActor* Traveler) const;
 UFUNCTION(BlueprintCallable,Category="Askir|World") bool Travel(AActor* Traveler);
 virtual bool Interact_Implementation(AActor* Interactor) override;
 virtual FText GetInteractionText_Implementation() const override;
protected:
 UFUNCTION() void OnOverlap(UPrimitiveComponent* Overlapped,AActor* Other,UPrimitiveComponent* OtherComp,int32 BodyIndex,bool bFromSweep,const FHitResult& Sweep);
private:
 bool bTransitioning=false;
};
