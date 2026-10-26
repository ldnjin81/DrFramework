#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Effects/DrEffectTypes.h"
#include "DrAbilitySystemComponent.generated.h"
class UDrTableSubsystem;
UCLASS(ClassGroup=(Dr), meta=(BlueprintSpawnableComponent))
class DRFRAMEWORK_API UDrAbilitySystemComponent : public UAbilitySystemComponent {
 GENERATED_BODY()
public:
 UDrAbilitySystemComponent();
 virtual bool GetShouldTick() const override { return true; }
 virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
 virtual void ApplyStatusEffect(const FDrResolvedEffect& Effect,AActor* Causer);
 virtual bool ApplyEffectById(int32 EffectId,UDrAbilitySystemComponent* TargetASC,AActor* Causer,const UDrTableSubsystem* Tables=nullptr);
 void AddPermanentEffect(const FDrResolvedEffect& Effect);
 void RemovePermanentEffect(const FGameplayTag& StateTag);
 float GetMoveSpeedMultiplier() const;
 float GetIncomingDamageMultiplier() const;
 float GetOutgoingDamageMultiplier() const;
 int32 GetStackCount(const FGameplayTag& StateTag) const;
 float AbsorbWithShield(float Damage);
 bool TryActivateAbilityByTag(const FGameplayTag& AbilityTag);
 void SetLooseState(const FGameplayTag& StateTag,bool bEnabled);
 void StartCooldown(const FGameplayTag& CooldownTag,float Duration);
 float GetCooldownRemaining(const FGameplayTag& CooldownTag) const;
protected:
 // 게임이 속성과 피해 적용 방식을 제공한다.
 virtual void ApplyDirectDamage(float Damage,AActor* Causer) {}
 virtual float GetCurrentHealth() const { return 0; }
 virtual void OnEffectApplied(const FDrResolvedEffect& Effect,AActor* Causer) {}
 virtual bool IsShieldEffect(const FDrResolvedEffect& Effect) const { return false; }
 void RemoveTimedEffect(const FGameplayTag& Tag);
 TMap<FGameplayTag,FDrResolvedEffect> TimedEffects;
private:
 void TickActiveAbilities(float DeltaTime);
 void TickTimedEffects(float DeltaTime);
 void TickCooldowns(float DeltaTime);
 float AggregateMultiplier(float FDrResolvedEffect::*Field) const;
 TMap<FGameplayTag,FDrResolvedEffect> BaseTimedEffects;
 TMap<FGameplayTag,int32> StackCounts;
 TMap<FGameplayTag,FDrResolvedEffect> PermanentEffects;
 struct FEffectClock { float Remaining=0; float SinceTick=0; };
 TMap<FGameplayTag,FEffectClock> EffectClocks;
 TMap<FGameplayTag,float> CooldownRemaining;
 TMap<FGameplayTag,TWeakObjectPtr<AActor>> StatusTickCausers;
 float ShieldPool=0;
 FGameplayTag ShieldTag;
};
