#include "Production/AskirProductionLibrary.h"
#include "Engine/DataTable.h"
bool UAskirProductionLibrary::GetQuestRow(const UDataTable* Table, FName Id, FAskirProductionQuestRow& Result)
{
    if (!Table || Id.IsNone()) return false;
    const FAskirProductionQuestRow* Row = Table->FindRow<FAskirProductionQuestRow>(Id, TEXT("QuestLookup"));
    if (!Row) return false;
    Result = *Row; return true;
}
bool UAskirProductionLibrary::GetSpellRow(const UDataTable* Table, FName Id, FAskirProductionSpellRow& Result)
{
    if (!Table || Id.IsNone()) return false;
    const FAskirProductionSpellRow* Row = Table->FindRow<FAskirProductionSpellRow>(Id, TEXT("SpellLookup"));
    if (!Row) return false;
    Result = *Row; return true;
}
bool UAskirProductionLibrary::GetItemRow(const UDataTable* Table, FName Id, FAskirProductionItemRow& Result)
{
    if (!Table || Id.IsNone()) return false;
    const FAskirProductionItemRow* Row = Table->FindRow<FAskirProductionItemRow>(Id, TEXT("ItemLookup"));
    if (!Row) return false;
    Result = *Row; return true;
}
int32 UAskirProductionLibrary::ExperienceForLevel(int32 Level)
{
    const int64 L = FMath::Clamp<int64>(Level, 1, 100);
    return static_cast<int32>(FMath::Min<int64>(2000000000LL, 100LL * L * L + 50LL * L));
}
