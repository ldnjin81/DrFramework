#pragma once
#include "Abilities/DrAbilitySystemComponent.h"
#include "DsAbilitySystemComponent.generated.h"
UCLASS(ClassGroup=(Ds),meta=(BlueprintSpawnableComponent))
class DRSAMPLE_API UDsAbilitySystemComponent : public UDrAbilitySystemComponent {
 GENERATED_BODY()
 friend class FDsEffectSystemTestAccess;
public:
 virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
 void ApplyDamage(UAbilitySystemComponent* TargetASC,float Damage,float PoiseDamage,AActor* Causer);
 void SpendStamina(float Amount);
 float GetStamina() const;
 void PushStaminaRegenDelay(float Seconds);
 void ApplyHeal(float Amount);
 void TakePoiseDamage(float Amount,AActor* Causer);
protected:
 virtual void ApplyDirectDamage(float Damage,AActor* Causer) override;
 virtual float GetCurrentHealth() const override;
 virtual void OnEffectApplied(const FDrResolvedEffect& Effect, AActor* Causer) override;
 virtual bool IsShieldEffect(const FDrResolvedEffect& Effect) const override;
private:
 void ApplyDirectPoiseDamage(float Damage,AActor* Causer);
 float StaminaRegenDelayRemaining=0;
};
