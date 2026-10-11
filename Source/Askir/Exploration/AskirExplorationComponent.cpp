#include "AskirExplorationComponent.h"

void UAskirExplorationComponent::DiscoverSite() { ++DiscoveredSites; }
void UAskirExplorationComponent::ReadLore() { ++LoreKnowledge; }
void UAskirExplorationComponent::Survey(float Amount) { SurveyProgress=FMath::Clamp(SurveyProgress+Amount,0.f,100.f); }
void UAskirExplorationComponent::ResetSurvey() { SurveyProgress=0.f; }
void UAskirExplorationComponent::StudyRuins(int32 Hours) { LoreKnowledge+=FMath::Max(0,Hours); }
