#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirProgressionComponent.generated.h"

UENUM(BlueprintType) enum class EAskirAttribute : uint8 { Strength, Agility, Vitality, Intellect, Willpower };
USTRUCT(BlueprintType)
struct FAskirProgressionSnapshot
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Level=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Experience=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 UnspentPoints=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Strength=5;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Agility=5;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Vitality=5;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Intellect=5;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Willpower=5;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TMap<FName,int32> SkillRanks;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TSet<FName> UnlockedPerks;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TMap<FName,int32> CraftingKnowledge;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TMap<FName,int32> Reputation;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAskirProgressionChanged);
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirProgressionComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UAskirProgressionComponent();
 UPROPERTY(EditAnywhere,BlueprintReadOnly,SaveGame,Category="Askir|Progression") FAskirProgressionSnapshot Progress;
 UPROPERTY(BlueprintAssignable) FAskirProgressionChanged OnProgressionChanged;
 UFUNCTION(BlueprintCallable,Category="Askir|Progression") void AwardExperience(int32 Amount);
 UFUNCTION(BlueprintPure,Category="Askir|Progression") int32 ExperienceToNextLevel() const;
 UFUNCTION(BlueprintCallable,Category="Askir|Progression") bool SpendAttributePoint(EAskirAttribute Attribute);
 UFUNCTION(BlueprintCallable,Category="Askir|Progression") bool TrainSkill(FName Skill,int32 MaximumRank=10);
 UFUNCTION(BlueprintCallable,Category="Askir|Progression") bool UnlockPerk(FName Perk,FName RequiredPerk=NAME_None,int32 MinimumLevel=1);
 UFUNCTION(BlueprintCallable,Category="Askir|Progression") void LearnRecipe(FName Recipe);
 UFUNCTION(BlueprintPure,Category="Askir|Progression") bool KnowsRecipe(FName Recipe) const;
 UFUNCTION(BlueprintCallable,Category="Askir|Progression") void ChangeReputation(FName Faction,int32 Delta);
 UFUNCTION(BlueprintPure,Category="Askir|Progression") int32 GetReputation(FName Faction) const;
 UFUNCTION(BlueprintPure,Category="Askir|Progression") float GetAttributeBonus(EAskirAttribute Attribute) const;
 UFUNCTION(BlueprintPure,Category="Askir|Progression") float GetSkillSuccessChance(FName Skill,int32 Difficulty) const;
 UFUNCTION(BlueprintCallable,Category="Askir|Progression") void RestoreProgress(const FAskirProgressionSnapshot& Snapshot);
 UFUNCTION(BlueprintPure,Category="Askir|Progression") FAskirProgressionSnapshot CaptureProgress() const {return Progress;}
private:
 static int32 LevelThreshold(int32 Level);
};
