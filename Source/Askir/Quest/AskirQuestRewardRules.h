#pragma once
#include <cstdint>
#include <limits>

/** Arithmetic used by the actual quest reward commit, with no Unreal dependencies. */
namespace AskirQuestRewardRules
{
 struct Progress
 {
  std::int32_t Level=1;
  std::int32_t Experience=0;
  std::int32_t UnspentPoints=0;
 };

 inline std::int64_t Threshold(std::int32_t Level)
 {return 100LL*Level*Level;}

 /** Prepare the entire XP award. Failure leaves Result untouched. No reward is lost to saturation. */
 inline bool PrepareExperience(const Progress& Current,std::int32_t Award,Progress& Result)
 {
  if(Current.Level<1 || Current.Level>100 || Current.Experience<0 || Current.UnspentPoints<0 || Award<0)return false;
  if(Current.Level<100 && Current.Experience>=Threshold(Current.Level))return false;
  const std::int64_t Maximum=std::numeric_limits<std::int32_t>::max();
  std::int64_t Experience=std::int64_t(Current.Experience)+Award;
  std::int64_t Points=Current.UnspentPoints;
  std::int32_t Level=Current.Level;
  while(Level<100 && Experience>=Threshold(Level))
  {Experience-=Threshold(Level);++Level;++Points;}
  if(Experience>Maximum || Points>Maximum)return false;
  Result={Level,static_cast<std::int32_t>(Experience),static_cast<std::int32_t>(Points)};
  return true;
 }
}
