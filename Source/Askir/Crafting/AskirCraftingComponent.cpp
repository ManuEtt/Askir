#include "Crafting/AskirCraftingComponent.h"
#include "Core/AskirCheckedMath.h"
#include "Inventory/AskirItemData.h"
#include "Economy/AskirEconomyRules.h"
#include "Equipment/AskirEquipmentComponent.h"
#include "Game/AskirGameMode.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

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
const FAskirCraftingRecipe* UAskirCraftingComponent::FindRecipe(FName RecipeID) const
{
    if(RecipeID.IsNone())return nullptr;
    const FAskirCraftingRecipe* Recipe=nullptr;
    for(const auto& Entry:Recipes)if(Entry.RecipeID==RecipeID){if(Recipe)return nullptr;Recipe=&Entry;}
    return Recipe;
}
bool UAskirCraftingComponent::IsRecipeKnown(FName RecipeID) const
{
    const auto* Recipe=FindRecipe(RecipeID);
    return Recipe&&(UnlockedRecipes.Contains(RecipeID)||Recipe->bKnownByDefault);
}
bool UAskirCraftingComponent::PrepareRecipe(UAskirInventoryComponent* Inventory,FName RecipeID,int32 Quantity,TArray<FAskirInventoryEntry>& PreparedItems,int32& NewCraftedItems) const
{
    if(bCraftInProgress||!IsValid(Inventory)||!IsRecipeKnown(RecipeID))return false;
    if(GetWorld())
        if(auto* GameMode=Cast<AAskirGameMode>(GetWorld()->GetAuthGameMode()))
            if(GameMode->IsPersistenceBusy())return false;
    const FAskirCraftingRecipe* Recipe=FindRecipe(RecipeID);
    if(!Recipe||Recipe->Ingredients.IsEmpty()||!IsValid(Recipe->Result.Item)||Recipe->Result.Item->ItemID.IsNone()||Recipe->Result.Item->MaxStackSize<=0||
        !AskirEconomyRules::PrepareCraftProgress(CraftedItems,CraftSkill,Recipe->RequiredSkill,Quantity,NewCraftedItems))return false;
    TArray<FAskirInventoryEntry> Removed,Added;
    TMap<FName,UAskirItemData*> IngredientIDs;
    for(const auto& Ingredient:Recipe->Ingredients)
    {
        FAskirInventoryEntry Entry;Entry.Item=Ingredient.Item;
        if(!IsValid(Entry.Item)||Entry.Item->ItemID.IsNone()||Entry.Item->MaxStackSize<=0||!AskirCheckedMath::TryPositiveProduct(Ingredient.Quantity,Quantity,Entry.Quantity))return false;
        // Repeated requirements for the same asset aggregate in the inventory.
        // Distinct assets sharing an ID are ambiguous and cannot be consumed.
        UAskirItemData*& Existing=IngredientIDs.FindOrAdd(Entry.Item->ItemID);
        if(Existing&&Existing!=Entry.Item.Get())return false;
        Existing=Entry.Item.Get();
        if(Entry.Item->ItemID==Recipe->Result.Item->ItemID&&Entry.Item!=Recipe->Result.Item)return false;
        Removed.Add(Entry);
    }
    FAskirInventoryEntry Result;Result.Item=Recipe->Result.Item;
    if(!AskirCheckedMath::TryPositiveProduct(Recipe->Result.Quantity,Quantity,Result.Quantity))return false;
    Added.Add(Result);
    return Inventory->PrepareExchange(Removed,Added,PreparedItems);
}
bool UAskirCraftingComponent::CanCraftRecipe(UAskirInventoryComponent* Inventory,FName RecipeID,int32 Quantity) const
{
    TArray<FAskirInventoryEntry> Prepared;int32 NewCraftedItems=0;
    return PrepareRecipe(Inventory,RecipeID,Quantity,Prepared,NewCraftedItems);
}
bool UAskirCraftingComponent::CraftRecipe(UAskirInventoryComponent* Inventory,FName RecipeID,int32 Quantity)
{
    TArray<FAskirInventoryEntry> Prepared;int32 NewCraftedItems=0;
    if(!PrepareRecipe(Inventory,RecipeID,Quantity,Prepared,NewCraftedItems))return false;
    TGuardValue<bool> CraftGuard(bCraftInProgress,true);
    TGuardValue<bool> InventoryGuard(Inventory->bTransactionInProgress,true);
    Inventory->Items=MoveTemp(Prepared);
    CraftedItems=NewCraftedItems;
    if(AActor* Owner=Inventory->GetOwner())
        if(auto* Equipment=Owner->FindComponentByClass<UAskirEquipmentComponent>())Equipment->ReconcileWithInventory();
    Inventory->OnInventoryChanged.Broadcast();
    return true;
}
