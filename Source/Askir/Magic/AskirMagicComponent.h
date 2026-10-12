#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Magic/AskirMagicRules.h"
#include "AskirMagicComponent.generated.h"

USTRUCT(BlueprintType)
struct FAskirMagicSnapshot
{
    GENERATED_BODY()
    UPROPERTY(SaveGame,BlueprintReadWrite) float Mana=100.f;
    UPROPERTY(SaveGame,BlueprintReadWrite) FName SelectedSpellID=TEXT("feuer_1");
    UPROPERTY(SaveGame,BlueprintReadWrite) TMap<FName,float> CooldownsRemaining;
    UPROPERTY(SaveGame,BlueprintReadWrite) float ShieldCapacity=0.f;
    UPROPERTY(SaveGame,BlueprintReadWrite) float ShieldSecondsRemaining=0.f;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAskirSpellCast,FName,SpellID);

UCLASS(ClassGroup=(Askir),meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirMagicComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UAskirMagicComponent();
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Magic") float MaxMana=100.f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Magic") float ManaRegenPerSecond=7.f;
    UPROPERTY(BlueprintAssignable) FAskirSpellCast OnSpellCast;
    UFUNCTION(BlueprintCallable) bool CastSelected();
    UFUNCTION(BlueprintCallable) bool SelectSpell(FName SpellID);
    UFUNCTION(BlueprintCallable) void CycleSpell(int32 Direction=1);
    UFUNCTION(BlueprintPure) float GetMana() const {return Mana;}
    UFUNCTION(BlueprintPure) float GetMaxMana() const {return MaxMana;}
    UFUNCTION(BlueprintPure) float GetManaPercent() const {return MaxMana>0?Mana/MaxMana:0.f;}
    UFUNCTION(BlueprintPure) float GetShield() const;
    UFUNCTION(BlueprintPure) FName GetSelectedSpellID() const;
    UFUNCTION(BlueprintPure) FText GetSelectedSpellName() const;
    UFUNCTION(BlueprintPure) float GetSelectedCooldownRemaining() const;
    UFUNCTION(BlueprintPure) FText GetLastFailureText() const;
    bool IsCasting() const;
    void CancelCasting();
    float AbsorbIncomingDamage(float Damage,bool bBypass=false);
    FAskirMagicSnapshot CaptureSnapshot() const;
    bool CanRestoreSnapshot(const FAskirMagicSnapshot& Snapshot) const;
    static bool ValidateSnapshotData(const FAskirMagicSnapshot& Snapshot,float ManaCapacity);
    bool RestoreSnapshot(const FAskirMagicSnapshot& Snapshot);
    static bool IsValidTarget(AActor* Caster,AActor* Target);
    static float ApplySpellHit(AActor* Caster,AActor* Target,AskirMagicRules::Element School,float Power,const FVector& Location);
private:
    double Now() const;
    float Mana=100.f;
    int32 SelectedIndex=0;
    TMap<FName,double> ReadyAt;
    double CastingUntil=-1.0;
    float ShieldCapacity=0.f;
    double ShieldUntil=-1.0;
    bool bCastTransaction=false;
    AskirMagicRules::Failure LastFailure=AskirMagicRules::Failure::None;
};
