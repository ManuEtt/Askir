#pragma once

class AAskirCharacter;
class UAskirMerchantComponent;

/** Configure the original Dornfeld starter economy only after actual imported assets exist. */
namespace AskirVillageEconomyCatalog
{
    bool ConfigurePlayer(AAskirCharacter* Player);
    bool ConfigureMerchant(UAskirMerchantComponent* Merchant);
}
