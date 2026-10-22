#pragma once
#include <algorithm>
#include <string>
#include <vector>

/** Derived SQ_003 view state. Durable quest receipts remain the only authority. */
namespace AskirHerbConversationRules
{
inline std::string FoldASCII(std::string Value)
{
    for(char& Character:Value)if(Character>='A'&&Character<='Z')Character=static_cast<char>(Character-'A'+'a');
    return Value;
}
inline bool IsVirtualStageFlag(const std::string& Flag)
{
    return FoldASCII(Flag).rfind("sq_003_stage_",0)==0;
}
inline std::vector<std::string> EffectiveFlags(const std::vector<std::string>& Durable,int Stage)
{
    std::vector<std::string> Result;Result.reserve(Durable.size()+1);
    for(const auto& Flag:Durable)if(!IsVirtualStageFlag(Flag))Result.push_back(Flag);
    static const char* const StageFlags[]={"sq_003_stage_available","sq_003_stage_explore","sq_003_stage_return","sq_003_stage_completed"};
    if(Stage>=0&&Stage<4)Result.emplace_back(StageFlags[Stage]);
    return Result;
}
inline bool IsHerbAction(const std::string& Action,const std::string& Payload)
{
    return Action=="accept_herbs"||Action=="return_herbs"||(Action=="quest_start"&&Payload=="sq_003");
}
inline bool ActionReady(const std::string& Action,const std::string& Payload,int Stage,bool TrustedGiver,bool DomainReady)
{
    if(!TrustedGiver||!DomainReady)return false;
    if(Action=="accept_herbs")return Stage==0&&(Payload=="help"||Payload=="bargain");
    if(Action=="return_herbs")return Stage==2&&Payload.empty();
    if(Action=="quest_start")return Stage==0&&Payload=="sq_003";
    return false;
}
}
