#pragma once
#include <cmath>
#include <cstdint>
#include <limits>

// Pure arithmetic used by gameplay transactions and the portable regression suite.
namespace AskirCheckedMath
{
    inline bool TryPositiveProduct(std::int32_t Unit, std::int32_t Quantity, std::int32_t& Result)
    {
        const std::int64_t Total = std::int64_t(Unit) * Quantity;
        if (Unit <= 0 || Quantity <= 0 || Total > std::numeric_limits<std::int32_t>::max()) return false;
        Result = static_cast<std::int32_t>(Total);
        return true;
    }

    inline bool CanCredit(std::int32_t Balance, std::int32_t Amount)
    {
        return Balance >= 0 && Amount >= 0 &&
            std::int64_t(Balance) + Amount <= std::numeric_limits<std::int32_t>::max();
    }

    inline std::int32_t SaturatingNonNegativeAdd(std::int32_t Value, std::int32_t Amount)
    {
        const std::int64_t Base = Value < 0 ? 0 : Value;
        const std::int64_t Delta = Amount < 0 ? 0 : Amount;
        const std::int64_t Total = Base + Delta;
        return static_cast<std::int32_t>(Total > std::numeric_limits<std::int32_t>::max() ?
            std::numeric_limits<std::int32_t>::max() : Total);
    }

    inline std::int32_t ClampAdd(std::int32_t Value, std::int32_t Delta, std::int32_t Minimum, std::int32_t Maximum)
    {
        const std::int64_t Total = std::int64_t(Value) + Delta;
        return static_cast<std::int32_t>(Total < Minimum ? Minimum : (Total > Maximum ? Maximum : Total));
    }

    inline bool TryCeilGold(double Price, std::int32_t Quantity, std::int32_t& Result)
    {
        if (!std::isfinite(Price) || Price <= 0.0 || Quantity <= 0) return false;
        const double Total = std::ceil(Price * double(Quantity));
        if (!std::isfinite(Total) || Total > double(std::numeric_limits<std::int32_t>::max())) return false;
        Result = static_cast<std::int32_t>(Total);
        return Result > 0;
    }
}
