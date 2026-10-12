#include "Survival/AskirSurvivalComponent.h"
#include "Equipment/AskirEquipmentComponent.h"
#include "Stealth/AskirStealthComponent.h"
#include "Morale/AskirMoraleComponent.h"
#include "Travel/AskirTravelComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "Camp/AskirCampComponent.h"
#include "Companion/AskirCompanionComponent.h"
#include "Exploration/AskirExplorationComponent.h"
#include "Crafting/AskirCraftingComponent.h"
#include "Simulation/AskirNeedsComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Characters/AskirCharacter.h"
#include "Core/AskirCheckedMath.h"
bool AAskirCharacter::EatFromCamp(float Amount)
{
    if(CampComponent&&SurvivalComponent&&CampComponent->Food>=Amount&&Amount>0){CampComponent->EatRations(Amount);SurvivalComponent->AddWarmth(Amount*.2f);return true;}return false;
}

bool AAskirCharacter::DrinkAndRecover(float Amount)
{
    if(Amount<=0)return false;if(NeedsComponent)NeedsComponent->SatisfyThirst(Amount);return true;
}

bool AAskirCharacter::WarmAtCamp(float Hours)
{
    if(Hours<=0||!CampComponent||CampComponent->Fuel<Hours)return false;CampComponent->BurnFuel(Hours);SurvivalComponent->RestAtFire(Hours);return true;
}

bool AAskirCharacter::ColdTravel(float Hours)
{
    if(Hours<=0)return false;SurvivalComponent->ApplyExposure(Hours);TravelComponent->Walk(Hours*100.f);return true;
}

bool AAskirCharacter::DryInShelter(float Amount)
{
    if(Amount<=0||CampComponent->Shelter<=0)return false;SurvivalComponent->DryOff(Amount);return true;
}

bool AAskirCharacter::DamageEquipmentOnHit(float Damage)
{
    if(Damage<=0)return false;EquipmentComponent->DamageGear(Damage*.1f);return true;
}

bool AAskirCharacter::RepairUsingMaterials(int32 MaterialsCost)
{
    if(MaterialsCost<=0||CraftingComponent->Materials<MaterialsCost)return false;CraftingComponent->ConsumeMaterials(MaterialsCost);EquipmentComponent->RepairGear(MaterialsCost*5.f);return true;
}

bool AAskirCharacter::EquipArmorForProtection(float Rating)
{
    if(Rating<0)return false;EquipmentComponent->EquipArmor(Rating);return true;
}

bool AAskirCharacter::EquipWeaponForDamage(float Rating)
{
    if(Rating<0)return false;EquipmentComponent->EquipWeapon(Rating);return true;
}

bool AAskirCharacter::SellDamagedGear(int32 GoldValue)
{
    if(GoldValue<=0||EquipmentComponent->Durability>25.f)return false;TradeComponent->EarnGold(GoldValue);EquipmentComponent->UnequipAll();return true;
}

bool AAskirCharacter::CrouchAndQuiet()
{
    StealthComponent->Crouch();StealthComponent->CalmDown(10.f);return true;
}

bool AAskirCharacter::RaiseAlarm(float Severity)
{
    if(Severity<=0)return false;StealthComponent->MakeNoise(Severity);StealthComponent->RaiseSuspicion(Severity);return true;
}

bool AAskirCharacter::SneakTravel(float Distance)
{
    if(Distance<=0)return false;TravelComponent->Walk(Distance);StealthComponent->CalmDown(2.f);return true;
}

bool AAskirCharacter::HideAtCamp()
{
    if(CampComponent->Shelter<=0)return false;StealthComponent->CalmDown(20.f);return true;
}

bool AAskirCharacter::RecoverAfterAlert(float Minutes)
{
    if(Minutes<=0)return false;StealthComponent->CalmDown(Minutes*.5f);return true;
}

bool AAskirCharacter::EncourageCompanion(float Amount)
{
    if(Amount<=0)return false;MoraleComponent->Encourage(Amount);CompanionComponent->Praise(Amount*.5f);return true;
}

bool AAskirCharacter::FearFromAmbush(float Amount)
{
    if(Amount<=0)return false;MoraleComponent->Intimidate(Amount);CompanionComponent->IncreaseStress(Amount);return true;
}

bool AAskirCharacter::RestMorale(float Hours)
{
    if(Hours<=0)return false;MoraleComponent->RecoverFear(Hours*8.f);CompanionComponent->Comfort(Hours*6.f);return true;
}

bool AAskirCharacter::TrainWithCompanion(float Hours)
{
    if(Hours<=0)return false;MoraleComponent->TrainCourage(Hours*2.f);CompanionComponent->TrainTogether(Hours);return true;
}

bool AAskirCharacter::LoseMoraleFromHunger(float Hunger)
{
    if(Hunger<=0)return false;MoraleComponent->BreakMorale(Hunger*.1f);return true;
}

bool AAskirCharacter::TravelWithSupplies(float Distance)
{
    if(Distance<=0||TravelComponent->Supplies<Distance*.01f)return false;TravelComponent->ConsumeSupplies(Distance*.01f);TravelComponent->Walk(Distance);return true;
}

bool AAskirCharacter::BuyTravelSupplies(int32 GoldCost)
{
    if(GoldCost<=0||TradeComponent->Gold<GoldCost)return false;TradeComponent->SpendGold(GoldCost);TravelComponent->Resupply(GoldCost*2.f);return true;
}

bool AAskirCharacter::RestAtCamp(float Hours)
{
    if(Hours<=0||CampComponent->Shelter<=0)return false;TravelComponent->Rest(Hours);MoraleComponent->RecoverFear(Hours);return true;
}

bool AAskirCharacter::FastTravelWithCost(float Distance)
{
    if(Distance<=0||TravelComponent->Supplies<Distance*.001f)return false;TravelComponent->FastTravel(Distance);return true;
}

bool AAskirCharacter::SurveyWhileTraveling(float Distance)
{
    if(Distance<=0)return false;TravelComponent->Walk(Distance);ExplorationComponent->Survey(Distance*.01f);return true;
}

bool AAskirCharacter::SellCraftedItem(int32 Price)
{
    if(Price<=0||CraftingComponent->CraftedItems<=0||!AskirCheckedMath::CanCredit(TradeComponent->Gold,Price))return false;--CraftingComponent->CraftedItems;TradeComponent->EarnGold(Price);TradeComponent->RecordTrade(Price);return true;
}

bool AAskirCharacter::BuyMaterials(int32 Count)
{
    int32 Price=0;
    if(!AskirCheckedMath::TryPositiveProduct(Count,2,Price)||!AskirCheckedMath::CanCredit(CraftingComponent->Materials,Count)||TradeComponent->Gold<Price)return false;
    TradeComponent->SpendGold(Price);
    CraftingComponent->GatherMaterials(Count);return true;
}

bool AAskirCharacter::BuyCampFuel(int32 Count)
{
    if(Count<=0||TradeComponent->Gold<Count)return false;TradeComponent->SpendGold(Count);CampComponent->GatherFuel(Count);return true;
}

bool AAskirCharacter::BuyCampFood(int32 Count)
{
    if(Count<=0||TradeComponent->Gold<Count)return false;TradeComponent->SpendGold(Count);CampComponent->StockFood(Count);return true;
}

bool AAskirCharacter::RewardExploration(int32 GoldReward)
{
    if(GoldReward<=0)return false;TradeComponent->EarnGold(GoldReward);ExplorationComponent->ReadLore();return true;
}

bool AAskirCharacter::BuildShelterFromMaterials(int32 Count)
{
    if(Count<=0||CraftingComponent->Materials<Count)return false;CraftingComponent->ConsumeMaterials(Count);CampComponent->BuildShelter(Count*5.f);return true;
}

bool AAskirCharacter::CollectWoodWhileTraveling(float Distance)
{
    if(Distance<=0)return false;TravelComponent->Walk(Distance);CampComponent->GatherFuel(Distance*.02f);return true;
}

bool AAskirCharacter::CookRations(float Amount)
{
    if(Amount<=0||CampComponent->Fuel<1.f||CampComponent->Food<Amount)return false;CampComponent->BurnFuel(1.f);CampComponent->EatRations(Amount);return true;
}

bool AAskirCharacter::ShelterCompanion(float Hours)
{
    if(Hours<=0||CampComponent->Shelter<=0)return false;CompanionComponent->Comfort(Hours*5.f);return true;
}

bool AAskirCharacter::BreakCamp()
{
    CampComponent->Shelter=0.f;return true;
}

bool AAskirCharacter::RewardCompanion(int32 Gold)
{
    if(Gold<=0||TradeComponent->Gold<Gold)return false;TradeComponent->SpendGold(Gold);CompanionComponent->Praise(Gold);return true;
}

bool AAskirCharacter::TrainCompanion(float Hours)
{
    if(Hours<=0)return false;CompanionComponent->TrainTogether(Hours);MoraleComponent->TrainCourage(Hours);return true;
}

bool AAskirCharacter::ComfortCompanionAtFire(float Hours)
{
    if(Hours<=0||CampComponent->Fuel<Hours)return false;CampComponent->BurnFuel(Hours);CompanionComponent->Comfort(Hours*10.f);return true;
}

bool AAskirCharacter::TravelTogether(float Distance)
{
    if(Distance<=0)return false;TravelComponent->Walk(Distance);CompanionComponent->TrainTogether(Distance*.001f);return true;
}

bool AAskirCharacter::CompanionUnderThreat(float Threat)
{
    if(Threat<=0)return false;CompanionComponent->IncreaseStress(Threat);MoraleComponent->Intimidate(Threat*.5f);return true;
}

bool AAskirCharacter::DiscoverAndReward(int32 Experience)
{
    if(Experience<0)return false;ExplorationComponent->DiscoverSite();if(ProgressionComponent)ProgressionComponent->AwardExperience(Experience);return true;
}

bool AAskirCharacter::StudyAndLearn(int32 Hours)
{
    if(Hours<=0)return false;ExplorationComponent->StudyRuins(Hours);if(ProgressionComponent)ProgressionComponent->AwardExperience(Hours*5);return true;
}

bool AAskirCharacter::SurveyWithSupplies(float Distance)
{
    if(Distance<=0||TravelComponent->Supplies<Distance*.01f)return false;TravelComponent->ConsumeSupplies(Distance*.01f);ExplorationComponent->Survey(Distance*.1f);return true;
}

bool AAskirCharacter::GatherAtDiscovery(int32 Amount)
{
    if(Amount<=0||ExplorationComponent->DiscoveredSites<=0)return false;CraftingComponent->GatherMaterials(Amount);return true;
}

bool AAskirCharacter::ShareLoreWithCompanion(int32 Lore)
{
    if(Lore<=0||ExplorationComponent->LoreKnowledge<Lore)return false;CompanionComponent->Praise(Lore);return true;
}

bool AAskirCharacter::CraftAndEarnExperience(int32 Cost)
{
    int32 Experience=0;
    if(!AskirCheckedMath::TryPositiveProduct(Cost,3,Experience)||CraftingComponent->Materials<Cost||!AskirCheckedMath::CanCredit(CraftingComponent->CraftedItems,1))return false;
    const int32 Before=CraftingComponent->CraftedItems;
    CraftingComponent->CraftItem(Cost);
    if(CraftingComponent->CraftedItems!=Before+1)return false;
    if(ProgressionComponent)ProgressionComponent->AwardExperience(Experience);return true;
}

bool AAskirCharacter::SalvageForCamp(int32 Amount)
{
    if(Amount<=0)return false;CraftingComponent->Salvage(Amount);CampComponent->GatherFuel(Amount*.25f);return true;
}

bool AAskirCharacter::TrainCraftingAtCamp()
{
    if(CampComponent->Shelter<=0)return false;CraftingComponent->ImproveCraftSkill();return true;
}

bool AAskirCharacter::CraftRepairKit(int32 Cost)
{
    if(Cost<=0||CraftingComponent->Materials<Cost)return false;CraftingComponent->ConsumeMaterials(Cost);EquipmentComponent->RepairGear(Cost*3.f);return true;
}

bool AAskirCharacter::TradeCraftForSupplies(int32 SupplyAmount)
{
    if(SupplyAmount<=0||CraftingComponent->CraftedItems<=0)return false;--CraftingComponent->CraftedItems;TravelComponent->Resupply(SupplyAmount);return true;
}
