#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AskirInteractable.h"
#include "AskirLootPickup.generated.h"
class USphereComponent;
class UStaticMeshComponent;
class UAskirItemData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAskirLootCollected, AActor*, Collector);
/** A world item that transfers its payload exactly once on successful interaction. */
UCLASS(Blueprintable)
class ASKIR_API AAskirLootPickup : public AActor, public IAskirInteractable
{
 GENERATED_BODY()
public:
 AAskirLootPickup();
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Askir|Loot") TObjectPtr<USphereComponent> InteractionSphere;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Askir|Loot") TObjectPtr<UStaticMeshComponent> Visual;
 UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Askir|Loot") TObjectPtr<UAskirItemData> Item;
 UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Askir|Loot", meta=(ClampMin="1")) int32 Quantity=1;
 UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Askir|Loot") FName QuestID=NAME_None;
 UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Askir|Loot") FName ObjectiveID=NAME_None;
 UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Askir|Loot") float MaximumDistance=225.f;
 /** Unique, persistent ID within a map. Leave None for non-persistent spawned loot. */
 UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Askir|Save") FName PersistentLootID=NAME_None;
 UFUNCTION(BlueprintCallable, Category="Askir|Save") void RestoreLootState(bool bConsumed);
 UPROPERTY(BlueprintAssignable, Category="Askir|Loot") FAskirLootCollected OnCollected;
 UFUNCTION(BlueprintPure, Category="Askir|Loot") bool IsCollected() const {return bCollected;}
 virtual bool Interact_Implementation(AActor* Interactor) override;
 virtual FText GetInteractionText_Implementation() const override;
protected:
 UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Askir|Loot") bool bCollected=false;
};
