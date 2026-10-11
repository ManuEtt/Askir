#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AskirProductionRows.h"
#include "AskirProductionLibrary.generated.h"
class UDataTable;
UCLASS()
class ASKIR_API UAskirProductionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Askir|Production")
    static bool GetQuestRow(const UDataTable* Table, FName Id, FAskirProductionQuestRow& Result);
    UFUNCTION(BlueprintCallable, Category="Askir|Production")
    static bool GetSpellRow(const UDataTable* Table, FName Id, FAskirProductionSpellRow& Result);
    UFUNCTION(BlueprintCallable, Category="Askir|Production")
    static bool GetItemRow(const UDataTable* Table, FName Id, FAskirProductionItemRow& Result);
    UFUNCTION(BlueprintPure, Category="Askir|Production")
    static int32 ExperienceForLevel(int32 Level);
};
