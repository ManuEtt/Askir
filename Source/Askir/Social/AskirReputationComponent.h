#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirReputationComponent.generated.h"
USTRUCT(BlueprintType)
struct FAskirFactionStanding
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName Faction=NAME_None;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Reputation=0;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAskirReputationChanged,FName,Faction,int32,NewReputation);
UCLASS(ClassGroup=(Askir),meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirReputationComponent:public UActorComponent
{
 GENERATED_BODY()
public:
 UAskirReputationComponent();
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Faction") TArray<FAskirFactionStanding> Standings;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Faction") int32 HostileBelow=-40;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Faction") int32 FriendlyFrom=40;
 UPROPERTY(BlueprintAssignable,Category="Askir|Faction") FAskirReputationChanged OnReputationChanged;
 UFUNCTION(BlueprintPure,Category="Askir|Faction") int32 GetReputation(FName Faction) const;
 UFUNCTION(BlueprintCallable,Category="Askir|Faction") int32 ChangeReputation(FName Faction,int32 Delta);
 UFUNCTION(BlueprintCallable,Category="Askir|Faction") void SetReputation(FName Faction,int32 Value);
 UFUNCTION(BlueprintPure,Category="Askir|Faction") bool IsHostile(FName Faction) const;
 UFUNCTION(BlueprintPure,Category="Askir|Faction") bool IsFriendly(FName Faction) const;
 UFUNCTION(BlueprintPure,Category="Askir|Faction") float GetMerchantPriceMultiplier(FName Faction) const;
};
