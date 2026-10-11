#include "AskirCraftingComponent.h"

void UAskirCraftingComponent::GatherMaterials(int32 Amount) { Materials+=FMath::Max(0,Amount); }
void UAskirCraftingComponent::CraftItem(int32 Cost) { if(Cost>0&&Materials>=Cost){Materials-=Cost;++CraftedItems;} }
void UAskirCraftingComponent::ImproveCraftSkill() { CraftSkill=FMath::Min(100,CraftSkill+1); }
void UAskirCraftingComponent::Salvage(int32 Amount) { Materials+=FMath::Max(0,Amount); }
void UAskirCraftingComponent::ConsumeMaterials(int32 Amount) { Materials=FMath::Max(0,Materials-FMath::Max(0,Amount)); }
