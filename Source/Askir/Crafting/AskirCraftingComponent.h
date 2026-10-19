#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "AskirCraftingComponent.generated.h"

USTRUCT(BlueprintType)
struct FAskirCraftingRecipe
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FName RecipeID=NAME_None;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FAskirInventoryEntry> Ingredients;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FAskirInventoryEntry Result;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,meta=(ClampMin="1",ClampMax="100")) int32 RequiredSkill=1;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bKnownByDefault=false;
};
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirCraftingComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Crafting") int32 Materials=0;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Crafting") int32 CraftedItems=0;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Crafting") int32 CraftSkill=1;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Crafting") TArray<FAskirCraftingRecipe> Recipes;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame,Category="Askir|Crafting") TArray<FName> UnlockedRecipes;
    UFUNCTION(BlueprintCallable,Category="Askir|Crafting") void GatherMaterials(int32 Amount);
    UFUNCTION(BlueprintCallable,Category="Askir|Crafting") void CraftItem(int32 Cost);
    UFUNCTION(BlueprintCallable,Category="Askir|Crafting") void ImproveCraftSkill();
    UFUNCTION(BlueprintCallable,Category="Askir|Crafting") void Salvage(int32 Amount);
    UFUNCTION(BlueprintCallable,Category="Askir|Crafting") void ConsumeMaterials(int32 Amount);
    UFUNCTION(BlueprintCallable,Category="Askir|Crafting") void UnlockRecipe(FName RecipeID);
    UFUNCTION(BlueprintPure,Category="Askir|Crafting") bool IsRecipeKnown(FName RecipeID) const;
    UFUNCTION(BlueprintCallable,Category="Askir|Crafting") bool CraftRecipe(UAskirInventoryComponent* Inventory,FName RecipeID,int32 Quantity=1);
    UFUNCTION(BlueprintPure,Category="Askir|Crafting") bool CanCraftRecipe(UAskirInventoryComponent* Inventory,FName RecipeID,int32 Quantity=1) const;
    const FAskirCraftingRecipe* FindRecipe(FName RecipeID) const;
    bool IsTransactionInProgress() const{return bCraftInProgress;}
private:
    bool PrepareRecipe(UAskirInventoryComponent* Inventory,FName RecipeID,int32 Quantity,TArray<FAskirInventoryEntry>& PreparedItems,int32& NewCraftedItems) const;
    bool bCraftInProgress=false;
};
