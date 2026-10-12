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
#endif
