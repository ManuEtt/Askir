#pragma once
#include "CoreMinimal.h"
#include "AskirTypes.generated.h"

USTRUCT(BlueprintType)
struct FAskirEntityID
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGuid Value;
    FAskirEntityID():Value(FGuid::NewGuid()){}
    bool IsValid() const { return Value.IsValid(); }
    friend uint32 GetTypeHash(const FAskirEntityID& V){return GetTypeHash(V.Value);}
    bool operator==(const FAskirEntityID& O)const{return Value==O.Value;}
};

USTRUCT(BlueprintType)
struct FAskirWorldTime
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Year=1;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Day=1;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Hour=8;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Minute=0;
    void AdvanceMinutes(int32 M)
    {
        Minute+=M; Hour+=Minute/60; Minute%=60;
        Day+=Hour/24; Hour%=24;
        Year+=(Day-1)/365; Day=((Day-1)%365)+1;
    }
};