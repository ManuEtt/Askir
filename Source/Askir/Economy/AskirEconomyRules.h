#pragma once
#include "Core/AskirCheckedMath.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

// The native merchant and inventory bridge both use these rules. No Unreal
// dependencies: boundary behaviour can be executed on a host without the editor.
namespace AskirEconomyRules
{
    inline std::int32_t UnitBuyPrice(std::int32_t BasePrice, double Multiplier)
    {
        if (BasePrice <= 0 || !std::isfinite(Multiplier) || Multiplier <= 0.0) return 0;
        const double Price = double(BasePrice) * std::clamp(Multiplier, 0.5, 2.0);
        return static_cast<std::int32_t>(std::clamp(std::floor(Price + 0.5),
            1.0, double(std::numeric_limits<std::int32_t>::max())));
    }

    inline std::int32_t UnitSellPrice(std::int32_t BasePrice, double Multiplier, double SellBackRatio)
    {
        const auto BuyPrice = UnitBuyPrice(BasePrice, Multiplier);
        if (BuyPrice <= 0 || !std::isfinite(SellBackRatio) || SellBackRatio <= 0.0) return 0;
        const double Price = double(BuyPrice) * std::clamp(SellBackRatio, 0.01, 1.0);
        return static_cast<std::int32_t>(std::clamp(std::floor(Price), 1.0, double(BuyPrice)));
    }

    enum class Direction : std::uint8_t { Buy, Sell };
    struct TradePlan
    {
        std::int32_t PlayerGold = 0;
        std::int32_t MerchantGold = 0;
        std::int32_t Stock = 0;
        std::int32_t TotalGold = 0;
    };

    inline bool PrepareTrade(Direction Operation, std::int32_t UnitPrice, std::int32_t Quantity,
        std::int32_t PlayerGold, std::int32_t MerchantGold, std::int32_t Stock, TradePlan& Output)
    {
        if (PlayerGold < 0 || MerchantGold < 0 || Stock < 0) return false;
        std::int32_t Total = 0;
        if (!AskirCheckedMath::TryPositiveProduct(UnitPrice, Quantity, Total)) return false;
        TradePlan Plan;
        switch (Operation)
        {
        case Direction::Buy:
            if (Stock < Quantity || PlayerGold < Total || !AskirCheckedMath::CanCredit(MerchantGold, Total)) return false;
            Plan = { PlayerGold - Total, MerchantGold + Total, Stock - Quantity, Total };
            break;
        case Direction::Sell:
            if (MerchantGold < Total || !AskirCheckedMath::CanCredit(PlayerGold, Total) ||
                !AskirCheckedMath::CanCredit(Stock, Quantity)) return false;
            Plan = { PlayerGold + Total, MerchantGold - Total, Stock + Quantity, Total };
            break;
        default:
            return false;
        }
        Output = Plan;
        return true;
    }

    inline bool PrepareCraftProgress(std::int32_t CraftedItems, std::int32_t Skill,
        std::int32_t RequiredSkill, std::int32_t Quantity, std::int32_t& Output)
    {
        if (Skill < 1 || Skill > 100 || RequiredSkill < 1 || RequiredSkill > 100 ||
            Skill < RequiredSkill || Quantity <= 0 || !AskirCheckedMath::CanCredit(CraftedItems, Quantity)) return false;
        Output = CraftedItems + Quantity;
        return true;
    }
}
