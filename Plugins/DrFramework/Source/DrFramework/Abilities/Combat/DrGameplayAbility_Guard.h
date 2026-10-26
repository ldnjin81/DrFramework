// 막기(쳐내기) 어빌리티를 선언한다.
#pragma once

#include "Abilities/DrGameplayAbility.h"
#include "DrGameplayAbility_Guard.generated.h"


/**
 * 발동 직후 짧은 쳐내기 구간(State.Parrying) 뒤 막기 구간(State.Guarding)이 이어진다.
 * 피해 경감 자체는 UDrAttributeSet이 두 태그를 보고 처리한다.
 */
UCLASS()
class DRFRAMEWORK_API UDrGameplayAbility_Guard : public UDrGameplayAbility {
  GENERATED_BODY()

public:
  UDrGameplayAbility_Guard();

  /** 수치·몽타주는 UDrPlayerCombatData::Guard에서 읽는다. */
  virtual float GetStaminaCost() const override;
  virtual void AbilityTick(float DeltaTime) override;

protected:
  virtual void ActivateAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo *ActorInfo,
      const FGameplayAbilityActivationInfo ActivationInfo,
      const FGameplayEventData *TriggerEventData) override;
  virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
                          const FGameplayAbilityActorInfo *ActorInfo,
                          const FGameplayAbilityActivationInfo ActivationInfo,
                          bool bReplicateEndAbility,
                          bool bWasCancelled) override;

private:
  float Elapsed = 0.f;
};
