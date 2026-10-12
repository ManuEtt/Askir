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

// Map serialization requires reflected struct identity to agree with its C++ hash.
template<> struct TStructOpsTypeTraits<FAskirEntityID> : TStructOpsTypeTraitsBase2<FAskirEntityID>
{
    enum { WithIdenticalViaEquality=true, WithGetTypeHash=true };
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
        if(M<=0)return;
        constexpr int64 MinutesPerYear=365LL*24*60;
        const int64 Current=(int64(FMath::Max(1,Year))-1)*MinutesPerYear+
            (int64(FMath::Clamp(Day,1,365))-1)*1440+
            int64(FMath::Clamp(Hour,0,23))*60+FMath::Clamp(Minute,0,59);
        const int64 Total=FMath::Min(Current+int64(M),int64(MAX_int32)*MinutesPerYear-1);
        Year=int32(Total/MinutesPerYear)+1;
        Day=int32((Total%MinutesPerYear)/1440)+1;
        Hour=int32((Total%1440)/60);
        Minute=int32(Total%60);
    }
};