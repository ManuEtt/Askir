#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirDailyRoutineComponent.generated.h"

UENUM(BlueprintType)
enum class EAskirDailyActivity : uint8 { Sleep, Work, Eat, Leisure, Guard };
USTRUCT(BlueprintType)
struct FAskirScheduleEntry
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 StartHour=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) EAskirDailyActivity Activity=EAskirDailyActivity::Sleep;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName DestinationTag=NAME_None;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAskirRoutineChanged,EAskirDailyActivity,Activity,FName,DestinationTag);
UCLASS(ClassGroup=(Askir),meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirDailyRoutineComponent:public UActorComponent
{
 GENERATED_BODY()
public:
 UAskirDailyRoutineComponent();
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Routine") TArray<FAskirScheduleEntry> Schedule;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Askir|Routine") EAskirDailyActivity CurrentActivity=EAskirDailyActivity::Sleep;
 UPROPERTY(BlueprintAssignable,Category="Askir|Routine") FAskirRoutineChanged OnRoutineChanged;
 UFUNCTION(BlueprintCallable,Category="Askir|Routine") void EvaluateAtHour(int32 Hour);
 UFUNCTION(BlueprintCallable,Category="Askir|Routine") void UseVillagerSchedule();
 UFUNCTION(BlueprintCallable,Category="Askir|Routine") void UseGuardSchedule();
 UFUNCTION(BlueprintPure,Category="Askir|Routine") FName GetDestinationTag() const{return CurrentDestinationTag;}
private:
 FName CurrentDestinationTag=NAME_None;
};
