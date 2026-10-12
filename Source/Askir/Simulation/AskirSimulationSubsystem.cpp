#include "Simulation/AskirSimulationSubsystem.h"
void UAskirSimulationSubsystem::AdvanceSimulation(int32 Minutes)
{
    if(Minutes<=0)return;
    State.Time.AdvanceMinutes(Minutes);
    if(State.SimulationTick<MAX_int64)++State.SimulationTick;
    OnSimulationAdvanced.Broadcast();
}
void UAskirSimulationSubsystem::RestoreWorldState(const FAskirWorldState& SavedState)
{
    State = SavedState;
    OnSimulationAdvanced.Broadcast();
}
