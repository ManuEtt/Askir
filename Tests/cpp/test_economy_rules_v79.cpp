#include "Economy/AskirEconomyRules.h"
#include <array>
#include <cassert>
#include <iostream>

int main()
{
    using namespace AskirEconomyRules;
    constexpr auto Limit=std::numeric_limits<std::int32_t>::max();
    constexpr auto Minimum=std::numeric_limits<std::int32_t>::min();
    const auto NaN=std::numeric_limits<double>::quiet_NaN();
    const auto Infinity=std::numeric_limits<double>::infinity();
    unsigned Checks=0;
    const std::array<std::int32_t,9> Bases={Minimum,-1,0,1,2,3,11,Limit-1,Limit};
    const std::array<double,13> Multipliers={-Infinity,-1,0,.001,.49,.5,.75,1,1.5,2,99,Infinity,NaN};
    const std::array<double,11> Ratios={-Infinity,-1,0,.00001,.01,.5,1,2,99,Infinity,NaN};
    for(const auto Base:Bases)for(const auto Multiplier:Multipliers)
    {
        const auto Buy=UnitBuyPrice(Base,Multiplier);
        const bool Valid=Base>0&&std::isfinite(Multiplier)&&Multiplier>0;
        assert((Buy>0)==Valid);++Checks;
        if(Valid)
        {
            const double Expected=std::floor(double(Base)*std::clamp(Multiplier,.5,2.0)+.5);
            assert(Buy==static_cast<std::int32_t>(std::clamp(Expected,1.0,double(Limit))));++Checks;
        }
        for(const auto Ratio:Ratios)
        {
            const auto Sell=UnitSellPrice(Base,Multiplier,Ratio);
            assert((Sell>0)==(Valid&&std::isfinite(Ratio)&&Ratio>0));++Checks;
            assert(Sell>=0&&Sell<=Buy);++Checks;
            // A buy/sell round trip at the same reputation cannot mint gold.
            if(Sell>0)assert(Sell-Buy<=0);
            ++Checks;
        }
    }
    const std::array<std::int32_t,8> Values={Minimum,-1,0,1,2,10,Limit-1,Limit};
    for(const auto Operation:{Direction::Buy,Direction::Sell})for(const auto Price:Values)
    for(const auto Quantity:Values)for(const auto Gold:Values)for(const auto MerchantGold:Values)
    for(const auto Stock:Values)
    {
        TradePlan Plan{17,19,23,29};
        const auto Total=std::int64_t(Price)*Quantity;
        bool Expected=Price>0&&Quantity>0&&Total<=Limit&&Gold>=0&&MerchantGold>=0&&Stock>=0;
        if(Expected&&Operation==Direction::Buy)
            Expected=Stock>=Quantity&&Gold>=Total&&std::int64_t(MerchantGold)+Total<=Limit;
        if(Expected&&Operation==Direction::Sell)
            Expected=MerchantGold>=Total&&std::int64_t(Gold)+Total<=Limit&&std::int64_t(Stock)+Quantity<=Limit;
        assert(PrepareTrade(Operation,Price,Quantity,Gold,MerchantGold,Stock,Plan)==Expected);++Checks;
        if(Expected)
        {
            assert(std::int64_t(Plan.PlayerGold)+Plan.MerchantGold==std::int64_t(Gold)+MerchantGold);++Checks;
            assert(Plan.PlayerGold>=0&&Plan.MerchantGold>=0&&Plan.Stock>=0&&Plan.TotalGold==Total);++Checks;
            assert(Plan.Stock==Stock+(Operation==Direction::Buy?-Quantity:Quantity));++Checks;
            assert(Plan.PlayerGold==Gold+(Operation==Direction::Buy?-Total:Total));++Checks;
        }
        else
        {
            assert(Plan.PlayerGold==17&&Plan.MerchantGold==19&&Plan.Stock==23&&Plan.TotalGold==29);++Checks;
        }
    }
    TradePlan Plan{17,19,23,29};
    assert(!PrepareTrade(static_cast<Direction>(255),1,1,10,10,10,Plan));++Checks;
    assert(Plan.PlayerGold==17&&Plan.MerchantGold==19&&Plan.Stock==23&&Plan.TotalGold==29);++Checks;
    const std::array<std::int32_t,8> Skills={Minimum,-1,0,1,2,99,100,101};
    for(const auto Crafted:Values)for(const auto Skill:Skills)for(const auto Required:Skills)for(const auto Quantity:Values)
    {
        std::int32_t Result=43;
        const bool Expected=Crafted>=0&&Skill>=1&&Skill<=100&&Required>=1&&Required<=100&&
            Skill>=Required&&Quantity>0&&std::int64_t(Crafted)+Quantity<=Limit;
        assert(PrepareCraftProgress(Crafted,Skill,Required,Quantity,Result)==Expected);++Checks;
        assert(Result==(Expected?std::int64_t(Crafted)+Quantity:43));++Checks;
    }
    std::cout<<"PASS: "<<Checks<<" assertions against production merchant, trade and crafting rules\n";
}
