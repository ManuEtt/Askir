#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Economy/AskirMerchantComponent.h"
#include "Economy/AskirTradeBridgeComponent.h"
#include "Economy/AskirEconomyComponent.h"
#include "Crafting/AskirCraftingComponent.h"

namespace
{
 FAskirInventoryEntry Entry(UAskirItemData* Item,int32 Count)
 {FAskirInventoryEntry Result;Result.Item=Item;Result.Quantity=Count;return Result;}
 UAskirItemData* Item(FName ID,int32 Stack=99)
 {auto* Result=NewObject<UAskirItemData>();Result->ItemID=ID;Result->MaxStackSize=Stack;return Result;}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirInventoryAtomicExchangeTest,"Askir.Inventory.AtomicExchange",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirInventoryAtomicExchangeTest::RunTest(const FString& Parameters)
{
 auto* Bag=NewObject<UAskirInventoryComponent>();
 auto* Ore=Item(TEXT("Ore"));auto* Sword=Item(TEXT("Sword"),1);
 Bag->MaxInventorySlots=2;
 TestTrue(TEXT("Seed original inventory"),Bag->AddItem(Ore,2));
 TestFalse(TEXT("Duplicate requirements aggregate before mutation"),Bag->ExchangeItems({Entry(Ore,1),Entry(Ore,2)},{Entry(Sword,1)}));
 TestEqual(TEXT("Insufficient requirements preserve ore"),Bag->GetItemCount(Ore),2);
 TestFalse(TEXT("Output cannot pay its own ingredient requirement"),Bag->ExchangeItems({Entry(Sword,1)},{Entry(Sword,2)}));
 TestFalse(TEXT("Capacity failure remains atomic"),Bag->ExchangeItems({Entry(Ore,2)},{Entry(Sword,3)}));
 TestEqual(TEXT("Capacity failure preserves ingredients"),Bag->GetItemCount(Ore),2);
 TestTrue(TEXT("Valid exchange can reuse released slots"),Bag->ExchangeItems({Entry(Ore,2)},{Entry(Sword,2)}));
 TestEqual(TEXT("Two output stacks"),Bag->Items.Num(),2);
 TestEqual(TEXT("All input consumed"),Bag->GetItemCount(Ore),0);
 TestFalse(TEXT("Negative quantity cannot satisfy HasItem"),Bag->HasItem(Sword,-1));
 Bag->Items.Reset();Ore->MaxStackSize=MAX_int32;
 TestTrue(TEXT("Maximum count fits one stack"),Bag->AddItem(Ore,MAX_int32));
 TestFalse(TEXT("Count overflow rejected"),Bag->AddItem(Ore,1));
 TestEqual(TEXT("Maximum count preserved"),Bag->GetItemCount(Ore),MAX_int32);
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirMerchantAtomicTradeTest,"Askir.Economy.AtomicInventoryTrade",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirMerchantAtomicTradeTest::RunTest(const FString& Parameters)
{
 auto* Merchant=NewObject<UAskirMerchantComponent>();auto* Bag=NewObject<UAskirInventoryComponent>();auto* Bridge=NewObject<UAskirTradeBridgeComponent>();
 auto* Sword=Item(TEXT("Sword"),1);
 FAskirShopItem Stock;Stock.ItemID=Sword->ItemID;Stock.BasePrice=10;Stock.Stock=3;
 Merchant->Inventory.Add(Stock);Merchant->MerchantGold=100;
 FAskirTradeCatalogEntry Catalog;Catalog.ItemID=Sword->ItemID;Catalog.Item=Sword;Bridge->Catalog.Add(Catalog);
 int32 Gold=50;Bag->MaxInventorySlots=1;
 TestFalse(TEXT("Capacity blocks purchase"),Bridge->BuyItem(Merchant,Bag,Sword->ItemID,2,Gold));
 TestEqual(TEXT("Failed purchase preserves gold"),Gold,50);
 TestEqual(TEXT("Failed purchase preserves stock"),Merchant->Inventory[0].Stock,3);
 TestEqual(TEXT("Failed purchase preserves merchant gold"),Merchant->MerchantGold,100);
 TestTrue(TEXT("Buy one item"),Bridge->BuyItem(Merchant,Bag,Sword->ItemID,1,Gold));
 TestEqual(TEXT("Buyer charged exact price"),Gold,40);
 TestEqual(TEXT("Buyer receives item"),Bag->GetItemCount(Sword),1);
 TestFalse(TEXT("Cannot sell missing quantity"),Bridge->SellItem(Merchant,Bag,Sword->ItemID,2,Gold));
 Merchant->MerchantGold=0;
 TestFalse(TEXT("Insolvent merchant leaves item untouched"),Bridge->SellItem(Merchant,Bag,Sword->ItemID,1,Gold));
 TestEqual(TEXT("Failed sale retains item"),Bag->GetItemCount(Sword),1);
 Merchant->MerchantGold=100;
 TestTrue(TEXT("Sell owned item"),Bridge->SellItem(Merchant,Bag,Sword->ItemID,1,Gold));
 TestEqual(TEXT("Sellback ratio applied"),Gold,45);
 TestEqual(TEXT("Sold item removed"),Bag->GetItemCount(Sword),0);
 TestEqual(TEXT("Stock restored"),Merchant->Inventory[0].Stock,3);
 Merchant->Inventory[0].BasePrice=MAX_int32;Merchant->SellBackRatio=1.f;
 TestEqual(TEXT("Sell price handles int32 boundary without float overflow"),Merchant->GetUnitSellPrice(Sword->ItemID),MAX_int32);
 auto* Market=NewObject<UAskirEconomyComponent>();
 FAskirMarketEntry Price;Price.ItemID=TEXT("Ore");Price.Stock=1;Price.CurrentPrice=.01f;Market->Market.Add(Price);Market->Gold=1;
 float Cost=0.f;
 TestTrue(TEXT("Fractional market purchase succeeds"),Market->Buy(Price.ItemID,1,Cost));
 TestEqual(TEXT("Fractional goods cannot be free"),Market->Gold,0);
 TestEqual(TEXT("Reported cost matches charge"),Cost,1.f);
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirCraftingAtomicRecipeTest,"Askir.Crafting.AtomicRecipe",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirCraftingAtomicRecipeTest::RunTest(const FString& Parameters)
{
 auto* Crafting=NewObject<UAskirCraftingComponent>();auto* Bag=NewObject<UAskirInventoryComponent>();
 auto* Ore=Item(TEXT("Ore"));auto* Sword=Item(TEXT("Sword"),1);
 FAskirCraftingRecipe Recipe;Recipe.RecipeID=TEXT("IronSword");Recipe.RequiredSkill=2;
 Recipe.Ingredients={Entry(Ore,1),Entry(Ore,2)};Recipe.Result=Entry(Sword,1);
 Crafting->Recipes.Add(Recipe);Bag->AddItem(Ore,2);
 TestFalse(TEXT("Locked recipe rejected"),Crafting->CraftRecipe(Bag,Recipe.RecipeID));
 Crafting->UnlockRecipe(Recipe.RecipeID);
 TestFalse(TEXT("Skill gate rejected"),Crafting->CraftRecipe(Bag,Recipe.RecipeID));
 Crafting->CraftSkill=2;
 TestFalse(TEXT("Repeated ingredient requirements cannot double-spend"),Crafting->CraftRecipe(Bag,Recipe.RecipeID));
 TestEqual(TEXT("Failed crafting preserves ore"),Bag->GetItemCount(Ore),2);
 Bag->AddItem(Ore,1);
 TestTrue(TEXT("Known recipe produces inventory item"),Crafting->CraftRecipe(Bag,Recipe.RecipeID));
 TestEqual(TEXT("Ingredients fully consumed"),Bag->GetItemCount(Ore),0);
 TestEqual(TEXT("Real crafted item exists"),Bag->GetItemCount(Sword),1);
 TestEqual(TEXT("Craft total updated"),Crafting->CraftedItems,1);
 TestFalse(TEXT("Overflowing batch rejected"),Crafting->CraftRecipe(Bag,Recipe.RecipeID,MAX_int32));
 TestEqual(TEXT("Rejected batch cannot alter totals"),Crafting->CraftedItems,1);
 Crafting->Materials=MAX_int32;Crafting->GatherMaterials(1);
 TestEqual(TEXT("GatherMaterials saturates"),Crafting->Materials,MAX_int32);
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirMerchantQuoteValidationTest,"Askir.Economy.QuotesAndMalformedCatalog",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirMerchantQuoteValidationTest::RunTest(const FString& Parameters)
{
 auto* Merchant=NewObject<UAskirMerchantComponent>();auto* Bag=NewObject<UAskirInventoryComponent>();auto* Bridge=NewObject<UAskirTradeBridgeComponent>();
 auto* Ore=Item(TEXT("QuoteOre"));
 FAskirShopItem Stock;Stock.ItemID=Ore->ItemID;Stock.BasePrice=11;Stock.Stock=3;Merchant->Inventory.Add(Stock);Merchant->MerchantGold=100;
 FAskirTradeCatalogEntry Catalog;Catalog.ItemID=Ore->ItemID;Catalog.Item=Ore;Bridge->Catalog.Add(Catalog);
 int32 Gold=50;
 TestTrue(TEXT("A quote succeeds without transfer"),Bridge->CanBuyItem(Merchant,Bag,Ore->ItemID,2,Gold));
 TestEqual(TEXT("Quote preserves wallet"),Gold,50);TestEqual(TEXT("Quote preserves stock"),Merchant->Inventory[0].Stock,3);
 TestEqual(TEXT("Quote preserves bag"),Bag->GetItemCount(Ore),0);
 FAskirShopItem InvalidStock;InvalidStock.ItemID=TEXT("MalformedOtherSKU");InvalidStock.Stock=-1;Merchant->Inventory.Add(InvalidStock);
 TestFalse(TEXT("Quote rejects malformed stock in another SKU"),Bridge->CanBuyItem(Merchant,Bag,Ore->ItemID,1,Gold));
 TestFalse(TEXT("Commit also rejects malformed stock in another SKU"),Bridge->BuyItem(Merchant,Bag,Ore->ItemID,1,Gold));
 Merchant->Inventory.RemoveAt(1);
 TestFalse(TEXT("Zero multiplier is invalid"),Bridge->CanBuyItem(Merchant,Bag,Ore->ItemID,1,Gold,0.f));
 TestFalse(TEXT("Negative multiplier is invalid"),Bridge->BuyItem(Merchant,Bag,Ore->ItemID,1,Gold,-1.f));
 TestFalse(TEXT("Cannot alias buyer and merchant wallets"),Bridge->BuyItem(Merchant,Bag,Ore->ItemID,1,Merchant->MerchantGold));
 TestEqual(TEXT("Rejected wallet alias preserves merchant"),Merchant->MerchantGold,100);
 Merchant->Inventory.Add(Stock);
 TestFalse(TEXT("Duplicate shop IDs cannot be priced or bought"),Bridge->BuyItem(Merchant,Bag,Ore->ItemID,1,Gold));
 TestEqual(TEXT("Duplicate shop price is unavailable"),Merchant->GetUnitBuyPrice(Ore->ItemID),0);
 Merchant->Inventory.RemoveAt(1);Bridge->Catalog.Add(Catalog);
 TestNull(TEXT("Duplicate catalog is ambiguous"),Bridge->FindItem(Ore->ItemID));
 TestFalse(TEXT("Ambiguous catalog cannot transfer"),Bridge->BuyItem(Merchant,Bag,Ore->ItemID,1,Gold));
 Bridge->Catalog.RemoveAt(1);
 TestTrue(TEXT("Valid purchase after malformed catalog rejection"),Bridge->BuyItem(Merchant,Bag,Ore->ItemID,2,Gold));
 TestEqual(TEXT("Committed wallet matches quote"),Gold,28);
 TestTrue(TEXT("Sale quote uses owned inventory"),Bridge->CanSellItem(Merchant,Bag,Ore->ItemID,2,Gold));
 TestEqual(TEXT("Sale quote preserves owned goods"),Bag->GetItemCount(Ore),2);
 TestFalse(TEXT("Sale quote cannot include missing goods"),Bridge->CanSellItem(Merchant,Bag,Ore->ItemID,3,Gold));
 TestFalse(TEXT("Wallet cannot alias an inventory stack that commit replaces"),Bridge->SellItem(Merchant,Bag,Ore->ItemID,1,Bag->Items[0].Quantity));
 TestEqual(TEXT("Rejected stack alias preserves owned goods"),Bag->GetItemCount(Ore),2);
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirCraftingQuoteValidationTest,"Askir.Crafting.QuotesAndMalformedRecipes",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirCraftingQuoteValidationTest::RunTest(const FString& Parameters)
{
 auto* Crafting=NewObject<UAskirCraftingComponent>();auto* Bag=NewObject<UAskirInventoryComponent>();
 auto* Ore=Item(TEXT("RecipeOre"));auto* DuplicateOre=Item(Ore->ItemID);auto* Sword=Item(TEXT("RecipeSword"),1);
 FAskirCraftingRecipe Recipe;Recipe.RecipeID=TEXT("QuoteSword");Recipe.bKnownByDefault=true;
 Recipe.Ingredients={Entry(Ore,2)};Recipe.Result=Entry(Sword,1);Crafting->Recipes.Add(Recipe);Bag->AddItem(Ore,4);
 TestTrue(TEXT("Valid quote includes ingredients and output capacity"),Crafting->CanCraftRecipe(Bag,Recipe.RecipeID,2));
 TestEqual(TEXT("Quote does not consume ingredients"),Bag->GetItemCount(Ore),4);
 TestEqual(TEXT("Quote does not produce output"),Bag->GetItemCount(Sword),0);TestEqual(TEXT("Quote does not alter total"),Crafting->CraftedItems,0);
 Crafting->Recipes[0].RequiredSkill=101;Crafting->CraftSkill=101;
 TestFalse(TEXT("Corrupt skill range cannot authorize craft"),Crafting->CraftRecipe(Bag,Recipe.RecipeID));
 Crafting->CraftSkill=1;Crafting->Recipes[0].RequiredSkill=1;
 Crafting->Recipes[0].Ingredients.Add(Entry(DuplicateOre,1));
 TestFalse(TEXT("Different assets sharing ingredient ID are ambiguous"),Crafting->CanCraftRecipe(Bag,Recipe.RecipeID));
 Crafting->Recipes[0].Ingredients.RemoveAt(1);Crafting->Recipes.Add(Recipe);
 TestNull(TEXT("Duplicate recipe ID is ambiguous"),Crafting->FindRecipe(Recipe.RecipeID));
 TestFalse(TEXT("Ambiguous recipe cannot be known"),Crafting->IsRecipeKnown(Recipe.RecipeID));
 Crafting->Recipes.RemoveAt(1);Bag->MaxInventorySlots=1;
 TestFalse(TEXT("Quote rejects insufficient output slots"),Crafting->CanCraftRecipe(Bag,Recipe.RecipeID,2));
 TestEqual(TEXT("Rejected quote preserves all ingredients"),Bag->GetItemCount(Ore),4);
 Bag->MaxInventorySlots=2;
 TestTrue(TEXT("Craft uses same validated plan as quote"),Crafting->CraftRecipe(Bag,Recipe.RecipeID,2));
 TestEqual(TEXT("Batch consumes inputs"),Bag->GetItemCount(Ore),0);TestEqual(TEXT("Batch produces two outputs"),Bag->GetItemCount(Sword),2);
 return true;
}
#endif
