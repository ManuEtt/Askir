#include "../../Source/Askir/Core/AskirCheckedMath.h"
#include <array>
#include <cassert>
#include <iostream>

int main()
{
    using namespace AskirCheckedMath;
    constexpr auto Limit=std::numeric_limits<std::int32_t>::max();
    constexpr auto Minimum=std::numeric_limits<std::int32_t>::min();
    const std::array<std::int32_t,10> Values={Minimum,-100,-1,0,1,2,100,46340,Limit-1,Limit};
    unsigned Checks=0;
    for(const auto Unit:Values)for(const auto Quantity:Values)
    {
        std::int32_t Result=42;
        const auto Expected=std::int64_t(Unit)*Quantity;
        const bool Valid=Unit>0&&Quantity>0&&Expected<=Limit;
        assert(TryPositiveProduct(Unit,Quantity,Result)==Valid);++Checks;
        assert(Result==(Valid?Expected:42));++Checks;
        assert(CanCredit(Unit,Quantity)==(Unit>=0&&Quantity>=0&&std::int64_t(Unit)+Quantity<=Limit));++Checks;
        const auto Sum=std::int64_t(Unit>0?Unit:0)+(Quantity>0?Quantity:0);
        assert(SaturatingNonNegativeAdd(Unit,Quantity)==(Sum>Limit?Limit:Sum));++Checks;
        const auto SignedSum=std::int64_t(Unit)+Quantity;
        assert(ClampAdd(Unit,Quantity,-100,100)==(SignedSum<-100?-100:SignedSum>100?100:SignedSum));++Checks;
    }
    std::int32_t Result=0;
    assert(TryCeilGold(.01,1,Result)&&Result==1);++Checks;
    assert(TryCeilGold(.1,3,Result)&&Result==1);++Checks;
    assert(TryCeilGold(1.25,2,Result)&&Result==3);++Checks;
    assert(TryCeilGold(Limit,1,Result)&&Result==Limit);++Checks;
    assert(!TryCeilGold(double(Limit)+.1,1,Result));++Checks;
    assert(!TryCeilGold(1.0,0,Result));++Checks;
    assert(!TryCeilGold(1.0,-1,Result));++Checks;
    assert(!TryCeilGold(-1.0,1,Result));++Checks;
    assert(!TryCeilGold(0.0,1,Result));++Checks;
    assert(!TryCeilGold(std::numeric_limits<double>::quiet_NaN(),1,Result));++Checks;
    assert(!TryCeilGold(std::numeric_limits<double>::infinity(),1,Result));++Checks;
    std::cout << "PASS: " << Checks << " assertions against gameplay checked arithmetic\n";
}
