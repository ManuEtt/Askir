#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AskirWorldPortal.generated.h"
class UBoxComponent;
/** A portal only accepts players and opens its configured, cooked map. */
UCLASS(Blueprintable)
class ASKIR_API AAskirWorldPortal : public AActor
{
 GENERATED_BODY()
public:
 AAskirWorldPortal();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|World") TObjectPtr<UBoxComponent> Trigger;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") FName DestinationMap=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|World") bool bEnabled=true;
 UFUNCTION(BlueprintCallable,Category="Askir|World") bool Travel(AActor* Traveler);
protected:
 UFUNCTION() void OnOverlap(UPrimitiveComponent* Overlapped,AActor* Other,UPrimitiveComponent* OtherComp,int32 BodyIndex,bool bFromSweep,const FHitResult& Sweep);
private:
 bool bTransitioning=false;
};
