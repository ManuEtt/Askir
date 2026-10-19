#include "Economy/AskirMerchantStateRules.h"
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
int Checks=0;
void Check(bool Condition,const char* Message)
{
    ++Checks;
    if(!Condition){std::cerr<<"FAILED: "<<Message<<'\n';std::exit(1);}
}
}
int main()
{
    using namespace AskirMerchantStateRules;
    const auto Max=std::numeric_limits<std::int32_t>::max();
    Check(ValidAmount(0),"Empty merchant wallet is valid");
    Check(ValidAmount(Max),"Maximum int32 amount is valid");
    Check(!ValidAmount(-1),"Negative stock or wallet is rejected");
    Check(!ValidAmount(std::int64_t(Max)+1),"Oversized serialized amount is rejected");
    Check(!ValidAmount(std::numeric_limits<std::int64_t>::min()),"INT64 minimum is rejected");
    Check(!ValidAmount(std::numeric_limits<std::int64_t>::max()),"INT64 maximum is rejected");
    Check(ValidSnapshot(0,0),"Empty shop is valid");
    Check(ValidSnapshot(Max,MaxItemsPerMerchant),"Bounded full shop is valid");
    Check(!ValidSnapshot(-1,1),"Invalid wallet invalidates entire snapshot");
    Check(!ValidSnapshot(1,MaxItemsPerMerchant+1),"Unbounded shop is rejected");
    Check(!ValidSnapshot(1,std::numeric_limits<std::size_t>::max()),"Wrapped count cannot pass");
    for(int Format=1;Format<=4;++Format)
    {
        Check(RestoreInitialState(Format,true),"Legacy saves ignore a fabricated new ledger");
        Check(RestoreInitialState(Format,false),"Legacy saves clear live merchant timeline");
    }
    Check(!RestoreInitialState(5,true),"V5 restores the saved stock");
    Check(RestoreInitialState(5,false),"Newly visited map uses initial stock");
    std::cout<<Checks<<" merchant-state assertions passed\n";
}
