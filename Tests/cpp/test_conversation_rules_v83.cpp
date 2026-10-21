#include "Dialogue/AskirConversationRules.h"
#include <cstdlib>
#include <iostream>

namespace
{
int Checks=0;
void Expect(bool Result,const char* Message)
{
    ++Checks;if(!Result){std::cerr<<"Conversation rule failed: "<<Message<<'\n';std::exit(1);}
}
AskirConversationRules::Graph ValidGraph()
{
    using namespace AskirConversationRules;
    Graph Result;Result.ID="d_test";Result.StartNode="start";
    Result.Nodes={{"start",{{"ask","answer",{},{}},{"leave","",{},{}}}},{"answer",{{"return","start",{},{}},{"end","",{},{}}}}};
    return Result;
}
}
int main()
{
    using namespace AskirConversationRules;
    Graph GraphData=ValidGraph();
    Expect(ValidateGraph(GraphData),"valid branching graph with deliberate return loop accepted");
    Expect(ChooseEntry(GraphData,0,{})=="start","no authored entry uses graph start");
    for(int Mutation=0;Mutation<13;++Mutation)
    {
        Graph Broken=GraphData;
        switch(Mutation)
        {
        case 0:Broken.ID.clear();break;
        case 1:Broken.StartNode.clear();break;
        case 2:Broken.StartNode="missing";break;
        case 3:Broken.Nodes.clear();break;
        case 4:Broken.Nodes[0].ID.clear();break;
        case 5:Broken.Nodes.push_back(Broken.Nodes[0]);break;
        case 6:Broken.Nodes[0].Choices[0].ID.clear();break;
        case 7:Broken.Nodes[0].Choices.push_back(Broken.Nodes[0].Choices[0]);break;
        case 8:Broken.Nodes[0].Choices[0].NextNode="missing";break;
        case 9:Broken.Entries.push_back({-2,{}, {},"start"});break;
        case 10:Broken.Entries.push_back({5,{}, {},"start"});break;
        case 11:Broken.Entries.push_back({-1,{}, {},"missing"});break;
        case 12:Broken.Nodes[0].Choices[0].RequiredFlags={""};break;
        }
        Expect(!ValidateGraph(Broken),"malformed graph rejected before installation");
        Expect(ChooseEntry(Broken,0,{}).empty(),"malformed graph cannot supply an executable start");
    }
    GraphData.Nodes[1].Choices[0].ID=GraphData.Nodes[0].Choices[0].ID;
    Expect(ValidateGraph(GraphData),"choice IDs are unique per node and protected across nodes by revision");
    GraphData=ValidGraph();GraphData.Nodes[1].Choices.clear();
    Expect(ValidateGraph(GraphData),"terminal node with no choices is valid and can be closed by UI");
    GraphData=ValidGraph();
    for(std::size_t Count=1;Count<=MaxNodes+2;++Count)
    {
        Graph Sized;Sized.ID="bounded";Sized.StartNode="n0";
        for(std::size_t N=0;N<Count;++N)Sized.Nodes.push_back({"n"+std::to_string(N),{}});
        Expect(ValidateGraph(Sized)==(Count<=MaxNodes),"node capacity is inclusive and fail-closed");
    }
    for(std::size_t Count=0;Count<=MaxChoicesPerNode+2;++Count)
    {
        Graph Sized=ValidGraph();Sized.Nodes[0].Choices.clear();
        for(std::size_t N=0;N<Count;++N)Sized.Nodes[0].Choices.push_back({"c"+std::to_string(N),"",{}, {}});
        Expect(ValidateGraph(Sized)==(Count<=MaxChoicesPerNode),"choice capacity is inclusive and fail-closed");
    }
    for(std::size_t Count=0;Count<=MaxEntries+2;++Count)
    {
        Graph Sized=ValidGraph();
        for(std::size_t N=0;N<Count;++N)Sized.Entries.push_back({-1,{}, {},"answer"});
        Expect(ValidateGraph(Sized)==(Count<=MaxEntries),"entry capacity is inclusive and fail-closed");
    }
    for(std::size_t Count=0;Count<=MaxFlagsPerCondition+2;++Count)
    {
        std::vector<std::string> Flags;
        for(std::size_t N=0;N<Count;++N)Flags.push_back("flag"+std::to_string(N));
        Expect(ValidFlagConditions(Flags,{})==(Count<=MaxFlagsPerCondition),"required flag capacity enforced");
        Expect(ValidFlagConditions({},Flags)==(Count<=MaxFlagsPerCondition),"forbidden flag capacity enforced");
    }
    Expect(!ValidFlagConditions({"known","known"},{}),"duplicate required condition rejected");
    Expect(!ValidFlagConditions({},{"known","known"}),"duplicate forbidden condition rejected");
    Expect(!ValidFlagConditions({"known"},{"known"}),"contradictory flag requirements rejected");
    Expect(!ValidFlagConditions({},{""}),"empty forbidden condition rejected");
    Expect(!ValidID(std::string(513,'x')),"oversized token rejected");
    for(unsigned RequiredMask=0;RequiredMask<16;++RequiredMask)
        for(unsigned ForbiddenMask=0;ForbiddenMask<16;++ForbiddenMask)
            for(unsigned ExistingMask=0;ExistingMask<16;++ExistingMask)
            {
                std::vector<std::string> Required,Forbidden,Existing;
                for(unsigned Bit=0;Bit<4;++Bit)
                {
                    const std::string Flag="f"+std::to_string(Bit);
                    if(RequiredMask&(1u<<Bit))Required.push_back(Flag);
                    if(ForbiddenMask&(1u<<Bit))Forbidden.push_back(Flag);
                    if(ExistingMask&(1u<<Bit))Existing.push_back(Flag);
                }
                const bool Expected=(RequiredMask&ForbiddenMask)==0&&
                    (RequiredMask&ExistingMask)==RequiredMask&&(ForbiddenMask&ExistingMask)==0;
                Expect(ConditionsMatch(Required,Forbidden,Existing)==Expected,"all required/forbidden/existing flag combinations match set algebra");
            }
    GraphData=ValidGraph();GraphData.Entries={{4,{"help"},{},"answer"},{4,{}, {},"start"},{-1,{"known"},{"blocked"},"answer"}};
    for(int Stage=0;Stage<=4;++Stage)for(int Mask=0;Mask<8;++Mask)
    {
        std::vector<std::string> Flags;
        if(Mask&1)Flags.push_back("help");
        if(Mask&2)Flags.push_back("known");
        if(Mask&4)Flags.push_back("blocked");
        const std::string Expected=Stage==4?((Mask&1)?"answer":"start"):((Mask&2)&&!(Mask&4)?"answer":"start");
        Expect(ChooseEntry(GraphData,Stage,Flags)==Expected,"first matching story entry wins without changing persistent flags");
    }
    for(int Stage=-2;Stage<=6;++Stage)for(bool Giver:{false,true})for(bool Ready:{false,true})
    {
        Expect(ActionReady("","",Stage,Giver,Ready),"plain dialogue needs no quest domain");
        Expect(!ActionReady("","unexpected",Stage,Giver,Ready),"payload without action cannot execute");
        for(const std::string Payload:{"help","bargain","invalid",""})
            Expect(ActionReady("accept_delivery",Payload,Stage,Giver,Ready)==(Giver&&Ready&&Stage==0&&(Payload=="help"||Payload=="bargain")),"delivery acceptance gates actor, stage and choice payload");
        Expect(ActionReady("return_delivery","",Stage,Giver,Ready)==(Giver&&Ready&&Stage==3),"delivery completion gates actor and return stage");
        Expect(!ActionReady("return_delivery","extra",Stage,Giver,Ready),"completion rejects malformed payload");
        for(int Quest=1;Quest<=24;++Quest)
        {
            const std::string Payload=Quest<10?"sq_00"+std::to_string(Quest):"sq_0"+std::to_string(Quest);
            Expect(ActionReady("quest_start",Payload,Stage,Giver,Ready)==(Giver&&Ready&&Stage==0&&Quest==1),"unimplemented quest actions cannot become executable through dialogue");
        }
        Expect(!ActionReady("grant_gold","100000",Stage,Giver,Ready),"unknown reward action cannot execute");
    }
    const std::vector<std::int64_t> Revisions={-1,0,1,19,std::numeric_limits<std::int64_t>::max()};
    for(unsigned Mask=0;Mask<8;++Mask)for(std::int64_t Revision:Revisions)
        for(std::int64_t Expected:Revisions)
        {
            const bool Active=(Mask&1)!=0,Busy=(Mask&2)!=0,Valid=(Mask&4)!=0;
            const bool Allowed=Active&&!Busy&&Valid&&Revision>0&&Revision==Expected&&Revision!=std::numeric_limits<std::int64_t>::max();
            Expect(CanSelect(Active,Busy,Valid,Revision,Expected)==Allowed,"stale, reentrant, inactive and invalid-context selections fail closed");
        }
    std::int64_t Revision=0,Next=0;
    for(int I=0;I<10000;++I)
    {
        const auto Old=Revision;Expect(NextRevision(Revision,Next),"normal choice/session token can advance");Revision=Next;
        Expect(Revision==Old+1,"revision never repeats across begin, choice and end boundaries");
        Expect(!CanSelect(true,false,true,Revision,Old),"previous button token is immediately obsolete");
    }
    Next=123;
    Expect(!NextRevision(-1,Next)&&Next==123,"negative revision rejected without mutation");
    Expect(!NextRevision(std::numeric_limits<std::int64_t>::max(),Next)&&Next==123,"revision exhaustion cannot wrap or alter output");
    Expect(NextRevision(std::numeric_limits<std::int64_t>::max()-1,Next)&&Next==std::numeric_limits<std::int64_t>::max(),"last positive revision remains representable");
    for(unsigned Mask=0;Mask<8;++Mask)
        Expect(ShouldCloseAfterAction((Mask&1)!=0,(Mask&2)!=0,(Mask&4)!=0)==(Mask!=6),"deferred close, destroyed context and cleared cursor cannot resume after a domain callback");
    std::cout<<"Conversation V83: "<<Checks<<" assertions passed\n";
}
