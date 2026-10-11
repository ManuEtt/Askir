#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirCraftingComponent.generated.h"

UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirCraftingComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Crafting") int32 Materials=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Crafting") int32 CraftedItems=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Crafting") int32 CraftSkill=1;
 UFUNCTION(BlueprintCallable,Category="Askir|Crafting") void GatherMaterials(int32 Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Crafting") void CraftItem(int32 Cost);
 UFUNCTION(BlueprintCallable,Category="Askir|Crafting") void ImproveCraftSkill();
 UFUNCTION(BlueprintCallable,Category="Askir|Crafting") void Salvage(int32 Amount);
 UFUNCTION(BlueprintCallable,Category="Askir|Crafting") void ConsumeMaterials(int32 Amount);
};
