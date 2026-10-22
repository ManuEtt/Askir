#include "Dialogue/AskirHerbConversationRules.h"
#include <iostream>

namespace
{
int Checks=0,Failures=0;
void Check(bool Value)
{
    ++Checks;if(!Value)++Failures;
}
bool Contains(const std::vector<std::string>& Values,const std::string& Value)
{return std::find(Values.begin(),Values.end(),Value)!=Values.end();}
}
int main()
{
    using namespace AskirHerbConversationRules;
    const char* const Variants[]={"sq_003_stage_return","SQ_003_STAGE_COMPLETED","Sq_003_StAgE_available","sq_003_stage_explore","sq_003_stage_forged","SQ_003_STAGE_"};
    const char* const Expected[]={"sq_003_stage_available","sq_003_stage_explore","sq_003_stage_return","sq_003_stage_completed"};
    for(unsigned Mask=0;Mask<64;++Mask)
    {
        std::vector<std::string> Durable={"rurik_well_lore","sq_001_choice_help","sq_003_choice_bargain","sq_003_stagecraft"};
        for(unsigned I=0;I<6;++I)if(Mask&(1U<<I))Durable.emplace_back(Variants[I]);
        const auto Before=Durable;
        for(int Stage=-2;Stage<6;++Stage)
        {
            const auto Effective=EffectiveFlags(Durable,Stage);
            const bool ValidStage=Stage>=0&&Stage<4;
            Check(Effective.size()==4U+(ValidStage?1U:0U));
            Check(Durable==Before);
            for(const auto& Lore:Before)if(!IsVirtualStageFlag(Lore))Check(Contains(Effective,Lore));
            int VirtualCount=0;for(const auto& Value:Effective)if(IsVirtualStageFlag(Value))++VirtualCount;
            Check(VirtualCount==(ValidStage?1:0));
            if(ValidStage)Check(Contains(Effective,Expected[Stage]));
        }
    }
    const char* const Actions[]={"accept_herbs","return_herbs","quest_start","accept_delivery","return_delivery","grant_gold",""};
    const char* const Payloads[]={"help","bargain","sq_003","sq_001","","forged"};
    for(const std::string Action:Actions)for(const std::string Payload:Payloads)
    {
        const bool Classified=Action=="accept_herbs"||Action=="return_herbs"||(Action=="quest_start"&&Payload=="sq_003");
        Check(IsHerbAction(Action,Payload)==Classified);
        for(int Stage=-1;Stage<5;++Stage)for(bool Trusted:{false,true})for(bool Ready:{false,true})
        {
            const bool ExpectedReady=Trusted&&Ready&&((Action=="accept_herbs"&&Stage==0&&(Payload=="help"||Payload=="bargain"))||
                (Action=="return_herbs"&&Stage==2&&Payload.empty())||(Action=="quest_start"&&Stage==0&&Payload=="sq_003"));
            Check(ActionReady(Action,Payload,Stage,Trusted,Ready)==ExpectedReady);
        }
    }
    std::cout<<"Herb conversation V84: "<<Checks<<" assertions, "<<Failures<<" failures\n";
    return Failures?1:0;
}
