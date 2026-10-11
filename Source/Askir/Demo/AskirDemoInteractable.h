#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AskirDemoInteractable.generated.h"
class UStaticMeshComponent;
UENUM(BlueprintType) enum class EAskirDemoObject : uint8 { Guide, Supplies, Campfire, Exit };
UCLASS()
class ASKIR_API AAskirDemoInteractable : public AActor
{
 GENERATED_BODY()
public:
 AAskirDemoInteractable();
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Mesh;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) EAskirDemoObject Kind=EAskirDemoObject::Supplies;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FText DisplayName;
 UPROPERTY(BlueprintReadOnly) bool bUsed=false;
 UFUNCTION(BlueprintCallable) void SetUsed(bool bNewUsed);
};
