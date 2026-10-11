#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AskirInteractable.h"
#include "AskirLootChest.generated.h"
class UStaticMeshComponent;
class UAskirItemData;
USTRUCT(BlueprintType)
struct FAskirChestEntry
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<UAskirItemData> Item=nullptr;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1")) int32 Quantity=1;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAskirChestOpened,AActor*, Opener);
/** Atomic all-or-nothing chest for the current unlimited-stack inventory. */
UCLASS(Blueprintable)
class ASKIR_API AAskirLootChest : public AActor, public IAskirInteractable
{
 GENERATED_BODY()
public:
 AAskirLootChest();
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Visual;
 UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Askir|Loot") TArray<FAskirChestEntry> Contents;
 UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Askir|Loot") float MaximumDistance=225.f;
 /** Unique, persistent ID within a map. Leave None for non-persistent spawned loot. */
 UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Askir|Save") FName PersistentLootID=NAME_None;
 UFUNCTION(BlueprintCallable, Category="Askir|Save") void RestoreLootState(bool bConsumed);
 UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Askir|Quest") FName QuestID=NAME_None;
 UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Askir|Quest") FName ObjectiveID=NAME_None;
 UPROPERTY(BlueprintAssignable, Category="Askir|Loot") FAskirChestOpened OnOpened;
 UFUNCTION(BlueprintPure, Category="Askir|Loot") bool IsOpened()const{return bOpened;}
 virtual bool Interact_Implementation(AActor* Interactor)override;
 virtual FText GetInteractionText_Implementation()const override;
private:
 UPROPERTY(VisibleInstanceOnly) bool bOpened=false;
};
