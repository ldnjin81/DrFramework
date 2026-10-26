// 이벤트 대상에게 데이터 기반 효과를 적용한다.
#include "Abilities/DrGameplayAbility_ApplyEffect.h"

#include "Abilities/DrAbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/Actor.h"

void UDrGameplayAbility_ApplyEffect::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  UDrAbilitySystemComponent *SourceASC = GetDrASC();
  if (!SourceASC) {
    EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
    return;
  }

  UDrAbilitySystemComponent *TargetASC = nullptr;
  if (TriggerEventData && TriggerEventData->Target)
    TargetASC =
        TriggerEventData->Target->FindComponentByClass<UDrAbilitySystemComponent>();

  const bool bApplied =
      SourceASC->ApplyEffectById(EffectId, TargetASC, GetDrActor());
  if (bApplied) {
    // 적용이 보장된 뒤 비용과 재사용 대기시간을 커밋한다.
    CommitCooldown();
  }
  EndAbility(Handle, ActorInfo, ActivationInfo, false, !bApplied);
}
