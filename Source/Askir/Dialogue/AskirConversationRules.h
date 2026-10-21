#pragma once
#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

/** Executed by the runtime before graph installation and before each choice commit. */
namespace AskirConversationRules
{
constexpr std::size_t MaxNodes=64,MaxChoicesPerNode=32,MaxEntries=32,MaxFlagsPerCondition=16;
struct Choice
{
    std::string ID,NextNode;
    std::vector<std::string> RequiredFlags,ForbiddenFlags;
};
struct Node {std::string ID;std::vector<Choice> Choices;};
struct Entry
{
    int DeliveryStage=-1;
    std::vector<std::string> RequiredFlags,ForbiddenFlags;
    std::string NodeID;
};
struct Graph
{
    std::string ID,StartNode;
    std::vector<Entry> Entries;
    std::vector<Node> Nodes;
};
inline bool ValidID(const std::string& ID) {return !ID.empty()&&ID.size()<=512;}
inline bool ValidFlagConditions(const std::vector<std::string>& Required,const std::vector<std::string>& Forbidden)
{
    if(Required.size()>MaxFlagsPerCondition||Forbidden.size()>MaxFlagsPerCondition)return false;
    std::unordered_set<std::string> RequiredSet,ForbiddenSet;
    for(const auto& Flag:Required)if(!ValidID(Flag)||!RequiredSet.insert(Flag).second)return false;
    for(const auto& Flag:Forbidden)
        if(!ValidID(Flag)||!ForbiddenSet.insert(Flag).second||RequiredSet.count(Flag))return false;
    return true;
}
inline bool ValidateGraph(const Graph& GraphData)
{
    if(!ValidID(GraphData.ID)||!ValidID(GraphData.StartNode)||GraphData.Nodes.empty()||
        GraphData.Nodes.size()>MaxNodes||GraphData.Entries.size()>MaxEntries)return false;
    std::unordered_set<std::string> Nodes;
    for(const auto& NodeData:GraphData.Nodes)
        if(!ValidID(NodeData.ID)||!Nodes.insert(NodeData.ID).second||NodeData.Choices.size()>MaxChoicesPerNode)return false;
    if(!Nodes.count(GraphData.StartNode))return false;
    for(const auto& NodeData:GraphData.Nodes)
    {
        std::unordered_set<std::string> Choices;
        for(const auto& ChoiceData:NodeData.Choices)
            if(!ValidID(ChoiceData.ID)||!Choices.insert(ChoiceData.ID).second||
                (!ChoiceData.NextNode.empty()&&!Nodes.count(ChoiceData.NextNode))||
                !ValidFlagConditions(ChoiceData.RequiredFlags,ChoiceData.ForbiddenFlags))return false;
    }
    for(const auto& EntryData:GraphData.Entries)
        if(EntryData.DeliveryStage< -1||EntryData.DeliveryStage>4||!Nodes.count(EntryData.NodeID)||
            !ValidFlagConditions(EntryData.RequiredFlags,EntryData.ForbiddenFlags))return false;
    return true;
}
inline bool ConditionsMatch(const std::vector<std::string>& Required,const std::vector<std::string>& Forbidden,
    const std::vector<std::string>& Existing)
{
    if(!ValidFlagConditions(Required,Forbidden))return false;
    const std::unordered_set<std::string> Flags(Existing.begin(),Existing.end());
    for(const auto& Flag:Required)if(!Flags.count(Flag))return false;
    for(const auto& Flag:Forbidden)if(Flags.count(Flag))return false;
    return true;
}
inline bool EntryMatches(const Entry& EntryData,int DeliveryStage,const std::vector<std::string>& Flags)
{
    return DeliveryStage>=0&&DeliveryStage<=4&&EntryData.DeliveryStage>=-1&&EntryData.DeliveryStage<=4&&
        (EntryData.DeliveryStage==-1||EntryData.DeliveryStage==DeliveryStage)&&
        ConditionsMatch(EntryData.RequiredFlags,EntryData.ForbiddenFlags,Flags);
}
inline std::string ChooseEntry(const Graph& GraphData,int DeliveryStage,const std::vector<std::string>& Flags)
{
    if(!ValidateGraph(GraphData))return {};
    for(const auto& EntryData:GraphData.Entries)if(EntryMatches(EntryData,DeliveryStage,Flags))return EntryData.NodeID;
    return GraphData.StartNode;
}
inline bool NextRevision(std::int64_t Current,std::int64_t& Out)
{
    if(Current<0||Current==std::numeric_limits<std::int64_t>::max())return false;
    Out=Current+1;return true;
}
inline bool CanSelect(bool Active,bool Busy,bool ContextValid,std::int64_t Revision,std::int64_t Expected)
{
    return Active&&!Busy&&ContextValid&&Revision>0&&Revision==Expected&&Revision<std::numeric_limits<std::int64_t>::max();
}
inline bool ShouldCloseAfterAction(bool EndRequested,bool ContextValid,bool CursorActive)
{
    return EndRequested||!ContextValid||!CursorActive;
}
/** Only implemented quest effects are callable. Unknown declarations remain disabled. */
inline bool ActionReady(const std::string& Action,const std::string& Payload,int Stage,bool DeliveryGiver,bool DomainReady)
{
    if(Action.empty())return Payload.empty();
    if(!DeliveryGiver||!DomainReady)return false;
    if(Action=="accept_delivery")return Stage==0&&(Payload=="help"||Payload=="bargain");
    if(Action=="return_delivery")return Stage==3&&Payload.empty();
    if(Action=="quest_start")return Stage==0&&Payload=="sq_001";
    return false;
}
}
