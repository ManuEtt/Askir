#include "Crafting/AskirCraftingComponent.h"
#include "Core/AskirCheckedMath.h"
#include "Inventory/AskirItemData.h"

void UAskirCraftingComponent::GatherMaterials(int32 Amount)
{if(!bCraftInProgress)Materials=AskirCheckedMath::SaturatingNonNegativeAdd(Materials,Amount);}
void UAskirCraftingComponent::CraftItem(int32 Cost)
{
    if(!bCraftInProgress&&Cost>0&&Materials>=Cost&&CraftedItems>=0&&CraftedItems<MAX_int32){Materials-=Cost;++CraftedItems;}
}
void UAskirCraftingComponent::ImproveCraftSkill()
{if(!bCraftInProgress)CraftSkill=AskirCheckedMath::ClampAdd(CraftSkill,1,1,100);}
void UAskirCraftingComponent::Salvage(int32 Amount){GatherMaterials(Amount);}
void UAskirCraftingComponent::ConsumeMaterials(int32 Amount)
{if(!bCraftInProgress)Materials=FMath::Max(0,FMath::Max(0,Materials)-FMath::Max(0,Amount));}
void UAskirCraftingComponent::UnlockRecipe(FName RecipeID)
{if(!bCraftInProgress&&!RecipeID.IsNone())UnlockedRecipes.AddUnique(RecipeID);}
bool UAskirCraftingComponent::IsRecipeKnown(FName RecipeID) const
{
    if(RecipeID.IsNone())return false;
    if(UnlockedRecipes.Contains(RecipeID))return true;
    for(const auto& Recipe:Recipes)if(Recipe.RecipeID==RecipeID)return Recipe.bKnownByDefault;
    return false;
}
bool UAskirCraftingComponent::CraftRecipe(UAskirInventoryComponent* Inventory,FName RecipeID,int32 Quantity)
{
    if(bCraftInProgress||!IsValid(Inventory)||Quantity<=0||!IsRecipeKnown(RecipeID)||!AskirCheckedMath::CanCredit(CraftedItems,Quantity))return false;
    const FAskirCraftingRecipe* Recipe=nullptr;
    for(const auto& Entry:Recipes)if(Entry.RecipeID==RecipeID){if(Recipe)return false;Recipe=&Entry;}
    if(!Recipe||Recipe->Ingredients.IsEmpty()||Recipe->RequiredSkill<1||CraftSkill<Recipe->RequiredSkill||!IsValid(Recipe->Result.Item))return false;
    TArray<FAskirInventoryEntry> Removed,Added;
    for(const auto& Ingredient:Recipe->Ingredients)
    {
        FAskirInventoryEntry Entry;Entry.Item=Ingredient.Item;
        if(!IsValid(Entry.Item)||!AskirCheckedMath::TryPositiveProduct(Ingredient.Quantity,Quantity,Entry.Quantity))return false;
        Removed.Add(Entry);
    }
    FAskirInventoryEntry Result;Result.Item=Recipe->Result.Item;
    if(!AskirCheckedMath::TryPositiveProduct(Recipe->Result.Quantity,Quantity,Result.Quantity))return false;
    Added.Add(Result);
    TArray<FAskirInventoryEntry> Prepared;
    if(!Inventory->PrepareExchange(Removed,Added,Prepared))return false;
    TGuardValue<bool> CraftGuard(bCraftInProgress,true);
    TGuardValue<bool> InventoryGuard(Inventory->bTransactionInProgress,true);
    Inventory->Items=MoveTemp(Prepared);
    CraftedItems+=Quantity;
    Inventory->OnInventoryChanged.Broadcast();
    return true;
}
