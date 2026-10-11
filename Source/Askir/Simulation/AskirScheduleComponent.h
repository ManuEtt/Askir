#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AskirScheduleComponent.generated.h"
UENUM(BlueprintType) enum class EAskirRoutineActivity:uint8{Sleep,Work,Eat,Drink,Socialize,Travel,Leisure,Guard,Flee,Idle};
USTRUCT(BlueprintType) struct FAskirScheduleEntry{GENERATED_BODY() UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 StartMinute=0,EndMinute=60; UPROPERTY(EditAnywhere,BlueprintReadWrite) EAskirRoutineActivity Activity=EAskirRoutineActivity::Idle; UPROPERTY(EditAnywhere,BlueprintReadWrite) FName LocationTag;};
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent)) class ASKIR_API UAskirScheduleComponent:public UActorComponent{GENERATED_BODY() public: UAskirScheduleComponent(); UFUNCTION(BlueprintPure) EAskirRoutineActivity GetActivityAtMinute(int32 M)const; UFUNCTION(BlueprintPure) FName GetLocationTagAtMinute(int32 M)const; UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FAskirScheduleEntry> Schedule;};
