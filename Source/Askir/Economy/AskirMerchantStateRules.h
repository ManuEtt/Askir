#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>

/** Bounds shared by the real SaveGame adapter and the portable regression test. */
namespace AskirMerchantStateRules
{
    constexpr std::size_t MaxMaps=1024;
    constexpr std::size_t MaxMerchantsPerMap=4096;
    constexpr std::size_t MaxItemsPerMerchant=4096;
    inline bool ValidAmount(std::int64_t Amount)
    {
        return Amount>=0 && Amount<=std::numeric_limits<std::int32_t>::max();
    }
    inline bool ValidSnapshot(std::int64_t Gold,std::size_t ItemCount)
    {
        return ValidAmount(Gold) && ItemCount<=MaxItemsPerMerchant;
    }
    inline bool RestoreInitialState(int FormatVersion,bool HasSavedMerchant)
    {
        // Versions 1-4 had no merchant ledger. Never keep live stock from the
        // timeline being replaced when an older save is loaded.
        return FormatVersion<5 || !HasSavedMerchant;
    }
}
