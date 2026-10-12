#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirEconomyComponent.generated.h"
USTRUCT(BlueprintType)struct FAskirMarketEntry{GENERATED_BODY()UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)FName ItemID;UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)int32 Stock=0;UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)float BasePrice=1;UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)float CurrentPrice=1;};
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))class ASKIR_API UAskirEconomyComponent:public UActorComponent{GENERATED_BODY()public:UAskirEconomyComponent();UFUNCTION(BlueprintCallable)void SimulateMarketDay();UFUNCTION(BlueprintPure)float GetPrice(FName ID)const;UFUNCTION(BlueprintCallable)bool Buy(FName ID,int32 Q,float&Cost);UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)int32 Gold=100;UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)TArray<FAskirMarketEntry> Market;};
