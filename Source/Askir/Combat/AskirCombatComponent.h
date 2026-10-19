#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/AskirDamageTypes.h"
#include "AskirCombatComponent.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAskirDamageTaken,const FAskirDamageInfo&,DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAskirDeath);
UCLASS(ClassGroup=(Askir),BlueprintType,meta=(BlueprintSpawnableComponent))
class ASKIR_API UAskirCombatComponent:public UActorComponent
{
    GENERATED_BODY()
public:
    UAskirCombatComponent();
    /** A native character block attempt consumes the sole guard opportunity for this hit. */
    UFUNCTION(BlueprintCallable) float ApplyDamage(const FAskirDamageInfo& Info,bool bSkipLegacyGuard=false);
    UFUNCTION(BlueprintCallable) void Heal(float Amount);
    UFUNCTION(BlueprintCallable, Category="Askir|Combat") void GrantDodgeInvulnerability(float Duration);
    UFUNCTION(BlueprintPure, Category="Askir|Combat") bool IsDodgeInvulnerable() const;
    UFUNCTION(BlueprintCallable) void Kill();
    UFUNCTION(BlueprintCallable) void ResetHealth();
    UFUNCTION(BlueprintCallable) void RestoreHealth(float SavedHealth);
    /** Encounter gating: inactive enemies cannot be damaged by nearby area attacks. */
    UFUNCTION(BlueprintCallable, Category="Askir|Combat") void SetDamageEnabled(bool bEnabled);
    UFUNCTION(BlueprintPure, Category="Askir|Combat") bool IsDamageEnabled() const { return bDamageEnabled; }
    UFUNCTION(BlueprintPure) float GetHealth()const{return Health;}
    UFUNCTION(BlueprintPure) float GetMaxHealth()const{return MaxHealth;}
    UFUNCTION(BlueprintPure) float GetHealthPercent()const;
    UFUNCTION(BlueprintPure) bool IsDead()const{return bDead;}
    /** Responsible actor resolved at the positive hit, including projectile instigators.
     * Returns a recent valid causer only; old hits cannot credit environmental deaths. */
    UFUNCTION(BlueprintPure,Category="Askir|Combat") AActor* GetLastDamageCauser() const;
    UFUNCTION(BlueprintPure,Category="Askir|Combat|Attribution") bool IsKillAttributedTo(AActor* Recipient) const;
    /** Time window in seconds for attributing a kill to a recent hit. */
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Askir|Combat|Attribution",meta=(ClampMin="0.0",ClampMax="60.0")) float KillCreditWindowSeconds=8.f;
    /** Explicitly clears attribution, e.g. on scripted environmental death. */
    UFUNCTION(BlueprintCallable,Category="Askir|Combat|Attribution") void ClearDamageAttribution();
    UPROPERTY(BlueprintAssignable) FAskirDamageTaken OnDamageTaken;
    UPROPERTY(BlueprintAssignable) FAskirDeath OnDeath;
protected:
    UPROPERTY(EditAnywhere) float MaxHealth=100;
    UPROPERTY(VisibleInstanceOnly) float Health=100;
    UPROPERTY(EditAnywhere) float PhysicalResistance=0;
    UPROPERTY(EditAnywhere) float MagicResistance=0;
private:
    TWeakObjectPtr<AActor> LastDamageCauser;
    double LastDamageTimeSeconds=-1.0;
    bool bDead=false;
    bool bDamageEnabled=true;
    float DodgeInvulnerableUntil=-1.f;
    static AActor* ResolveDamageRecipient(AActor* DamageSource);
};
