#include "Economy/AskirVillageEconomyCatalog.h"
#include "Characters/AskirCharacter.h"
#include "Crafting/AskirCraftingComponent.h"
#include "Economy/AskirMerchantComponent.h"
#include "Economy/AskirTradeBridgeComponent.h"
#include "Inventory/AskirItemData.h"
#include "Misc/PackageName.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    struct FAskirVillageItemSpec
    {
        const TCHAR* ID;
        int32 Price;
        int32 Stock;
        int32 MaxStock;
    };
    struct FAskirVillageRecipeSpec
    {
        const TCHAR* ID;
        int32 RequiredSkill;
        bool bKnownByDefault;
        const TCHAR* IngredientA;
        int32 QuantityA;
        const TCHAR* IngredientB;
        int32 QuantityB;
        const TCHAR* Result;
        int32 ResultQuantity;
    };
#include "Economy/AskirVillageEconomyCatalog.generated.inl"

    bool LoadVillageItems(TMap<FName, UAskirItemData*>& OutItems)
    {
        // Preflight all packages before loading: missing sources never emit failed-load noise.
        for (const auto& Spec : VillageItems)
        {
            const FString Package = FString::Printf(TEXT("/Game/Askir/V79/Items/%s"), Spec.ID);
            if (!FPackageName::DoesPackageExist(Package)) return false;
        }
        for (const auto& Spec : VillageItems)
        {
            const FString Package = FString::Printf(TEXT("/Game/Askir/V79/Items/%s"), Spec.ID);
            const FString ObjectPath = Package + TEXT(".") + Spec.ID;
            UAskirItemData* Item = LoadObject<UAskirItemData>(nullptr, *ObjectPath);
            if (!Item || Item->ItemID != FName(Spec.ID) || Item->MaxStackSize < 1) return false;
            OutItems.Add(Item->ItemID, Item);
        }
        return true;
    }
}

bool AskirVillageEconomyCatalog::ConfigurePlayer(AAskirCharacter* Player)
{
    if (!IsValid(Player) || !Player->TradeBridgeComponent || !Player->CraftingComponent) return false;
    const bool bNeedsCatalog = Player->TradeBridgeComponent->Catalog.IsEmpty();
    const bool bNeedsRecipes = Player->CraftingComponent->Recipes.IsEmpty();
    if (!bNeedsCatalog && !bNeedsRecipes) return true;
    TMap<FName, UAskirItemData*> Items;
    if (!LoadVillageItems(Items)) return false;
    TArray<FAskirTradeCatalogEntry> Catalog;
    for (const auto& Spec : VillageItems)
    {
        FAskirTradeCatalogEntry Entry;
        Entry.ItemID = FName(Spec.ID);
        Entry.Item = Items.FindChecked(Entry.ItemID);
        Catalog.Add(Entry);
    }
    TArray<FAskirCraftingRecipe> Recipes;
    for (const auto& Spec : VillageRecipes)
    {
        FAskirCraftingRecipe Recipe;
        Recipe.RecipeID = FName(Spec.ID);
        Recipe.RequiredSkill = Spec.RequiredSkill;
        Recipe.bKnownByDefault = Spec.bKnownByDefault;
        FAskirInventoryEntry Ingredient;
        Ingredient.Item = Items.FindChecked(FName(Spec.IngredientA));
        Ingredient.Quantity = Spec.QuantityA;
        Recipe.Ingredients.Add(Ingredient);
        if (Spec.QuantityB > 0)
        {
            Ingredient.Item = Items.FindChecked(FName(Spec.IngredientB));
            Ingredient.Quantity = Spec.QuantityB;
            Recipe.Ingredients.Add(Ingredient);
        }
        Recipe.Result.Item = Items.FindChecked(FName(Spec.Result));
        Recipe.Result.Quantity = Spec.ResultQuantity;
        Recipes.Add(Recipe);
    }
    // Commit only after every referenced package loaded; designer catalogs/recipes are retained.
    if (bNeedsCatalog) Player->TradeBridgeComponent->Catalog = MoveTemp(Catalog);
    if (bNeedsRecipes) Player->CraftingComponent->Recipes = MoveTemp(Recipes);
    return true;
}

bool AskirVillageEconomyCatalog::ConfigureMerchant(UAskirMerchantComponent* Merchant)
{
    if (!IsValid(Merchant)) return false;
    if (!Merchant->Inventory.IsEmpty()) return true;
    // Fallback defaults belong to this explicitly identified shop; other traders keep their design.
    if (Merchant->MerchantID != FName(VillageMerchantID)) return false;
    TMap<FName, UAskirItemData*> Items;
    if (!LoadVillageItems(Items)) return false;
    TArray<FAskirShopItem> Inventory;
    for (const auto& Spec : VillageItems)
    {
        FAskirShopItem Entry;
        Entry.ItemID = FName(Spec.ID);
        Entry.BasePrice = Spec.Price;
        Entry.Stock = Spec.Stock;
        Entry.MaxStock = Spec.MaxStock;
        Inventory.Add(Entry);
    }
    Merchant->Inventory = MoveTemp(Inventory);
    Merchant->MerchantGold = VillageMerchantGold;
    Merchant->SellBackRatio = VillageSellBackRatio;
    return true;
}
