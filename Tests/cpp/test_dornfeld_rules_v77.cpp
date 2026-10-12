#include "Source/Askir/Mission/AskirDornfeldRules.h"
#include <iostream>

namespace Rules = AskirDornfeldRules;
using Rules::Stage;
int failures=0, checks=0;
void check(bool result, const char* label)
{
 ++checks;
 if(!result){++failures;std::cerr << "FAIL: " << label << '\n';}
}
int main()
{
 check(Rules::GoldReward==90 && Rules::ExperienceReward==180,"Exact first-mission rewards");
 check(Rules::NormalizeStage(0,3,2,true,false)==Stage::NotStarted,"No accepted quest from stale progress");
 check(Rules::NormalizeStage(255,0,0,false,false)==Stage::NotStarted,"Reject invalid stage");
 for(unsigned saved=1;saved<=5;++saved)
 {
  check(Rules::NormalizeStage(saved,0,0,false,false)==Stage::CollectShards,"Acceptance retains shard stage");
  check(Rules::NormalizeStage(saved,2,2,true,false)==Stage::CollectShards,"Missing shard evidence rewinds stale guardian/chest stage");
  check(Rules::NormalizeStage(saved,3,0,true,false)==Stage::DefeatGuardians,"Missing guardian evidence rewinds stale chest stage");
  check(Rules::NormalizeStage(saved,3,2,false,false)==Stage::OpenChest,"Both guardians lead to locked chest stage");
  check(Rules::NormalizeStage(saved,3,2,true,false)==Stage::ReturnToElder,"Actual chest receipt allows turn-in");
 }
 for(unsigned stage=0;stage<=5;++stage)
 {
  check(Rules::NormalizeStage(stage,0,0,false,true)==Stage::Completed,"Reward receipt remains complete even with legacy missing IDs");
  check(Rules::CanCollectShard(static_cast<Stage>(stage),true,false,0)==(stage==1),"Shard gate uses correct stage");
  check(Rules::CanDefeatGuardian(static_cast<Stage>(stage),true,false,0)==(stage==2),"Guardian gate uses correct stage");
  check(Rules::CanOpenChest(static_cast<Stage>(stage),true)==(stage==3),"Chest gate uses correct stage");
  check(Rules::CanClaimReward(static_cast<Stage>(stage),false,true,true,0)==(stage==4),"Reward gate requires return stage");
 }
 check(!Rules::CanCollectShard(Stage::CollectShards,false,false,0),"Empty shard ID rejected");
 check(!Rules::CanCollectShard(Stage::CollectShards,true,true,1),"Duplicate shard rejected");
 check(!Rules::CanCollectShard(Stage::CollectShards,true,false,3),"A fourth shard is rejected");
 check(!Rules::CanCollectShard(Stage::CollectShards,true,false,-1),"Negative shard count rejected");
 check(!Rules::CanDefeatGuardian(Stage::DefeatGuardians,false,false,0),"Empty guardian ID rejected");
 check(!Rules::CanDefeatGuardian(Stage::DefeatGuardians,true,true,1),"Duplicate guardian rejected");
 check(!Rules::CanDefeatGuardian(Stage::DefeatGuardians,true,false,2),"A third guardian is rejected");
 check(!Rules::CanOpenChest(Stage::OpenChest,false),"Empty chest ID rejected");
 check(!Rules::CanClaimReward(Stage::ReturnToElder,true,true,true,0),"Reward receipt prevents a second grant");
 check(!Rules::CanClaimReward(Stage::ReturnToElder,false,false,true,0),"Missing wallet rejects without committing");
 check(!Rules::CanClaimReward(Stage::ReturnToElder,false,true,false,0),"Missing XP component rejects without committing");
 check(!Rules::CanClaimReward(Stage::ReturnToElder,false,true,true,-1),"Invalid wallet rejects");
 const auto max=std::numeric_limits<std::int32_t>::max();
 check(Rules::CanClaimReward(Stage::ReturnToElder,false,true,true,max-90),"Exact maximum gold fits");
 check(!Rules::CanClaimReward(Stage::ReturnToElder,false,true,true,max-89),"Reward gold overflow rejects");
 check(!Rules::CanClaimReward(Stage::ReturnToElder,false,true,true,std::numeric_limits<std::int64_t>::max()),"64-bit malicious gold rejects");
 std::cout << "Dornfeld production rules: " << checks << " checks, " << failures << " failures. Unreal runtime: NOT_EXECUTED.\n";
 return failures?1:0;
}
