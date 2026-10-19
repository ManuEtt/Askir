#include "Quest/AskirQuestRewardRules.h"
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
 int Checks=0;
 void Check(bool Condition){++Checks;if(!Condition){std::cerr<<"Quest reward assertion "<<Checks<<" failed\n";std::exit(1);}}
 std::int64_t Earned(const AskirQuestRewardRules::Progress& P)
 {
  std::int64_t Value=P.Experience;
  for(int Level=1;Level<P.Level;++Level)Value+=AskirQuestRewardRules::Threshold(Level);
  return Value;
 }
}

int main()
{
 using AskirQuestRewardRules::PrepareExperience;
 using AskirQuestRewardRules::Progress;
 const auto Max=std::numeric_limits<std::int32_t>::max();
 Progress Result{9,8,7};
 for(const Progress Bad: {Progress{0,0,0},Progress{101,0,0},Progress{1,-1,0},Progress{1,0,-1},Progress{1,100,0}})
 {
  Check(!PrepareExperience(Bad,1,Result));Check(Result.Level==9 && Result.Experience==8 && Result.UnspentPoints==7);
 }
 Check(!PrepareExperience({1,0,0},-1,Result));Check(Result.Level==9);
 Check(!PrepareExperience({100,Max,0},1,Result));Check(Result.Level==9);
 Check(!PrepareExperience({1,99,Max},1,Result));Check(Result.Level==9);
 Check(PrepareExperience({1,99,0},Max,Result));
 Check(Earned(Result)==std::int64_t(Max)+99); // Level costs are paid before the final int32 storage check.
 Check(Result.Level==100 && Result.UnspentPoints==99);
 for(int Level=1;Level<=100;++Level)
 {
  const auto Threshold=AskirQuestRewardRules::Threshold(Level);
  const int BaseXP=static_cast<int>(Threshold-1);
  for(int Award: {0,1,99,100,180,10000,1000000})
  {
   const Progress Before{Level,BaseXP,4};
   Check(PrepareExperience(Before,Award,Result));
   Check(Earned(Result)==Earned(Before)+Award);
   Check(Result.UnspentPoints==4+Result.Level-Level);
   Check(Result.Level>=Level && Result.Level<=100);
   Check(Result.Level==100 || Result.Experience<AskirQuestRewardRules::Threshold(Result.Level));
  }
 }
 Check(PrepareExperience({1,0,0},180,Result));
 Check(Result.Level==2 && Result.Experience==80 && Result.UnspentPoints==1);
 std::cout<<Checks<<" quest reward assertions passed\n";
}
