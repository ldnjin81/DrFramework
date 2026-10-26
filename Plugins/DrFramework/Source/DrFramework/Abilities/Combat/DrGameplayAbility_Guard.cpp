// 막기 어빌리티의 쳐내기·막기 구간 전환을 구현한다.
#include "Abilities/Combat/DrGameplayAbility_Guard.h"

#include "Abilities/DrAbilitySystemComponent.h"
#include "Combat/DrPlayerCombatData.h"
#include "Animation/AnimMontage.h"
#include "Core/DrGameplayTags.h"

UDrGameplayAbility_Guard::UDrGameplayAbility_Guard() {
  SetAssetTags(FGameplayTagContainer(DrTags::Ability_Guard));
  BlockDuringCombatStates();
}

float UDrGameplayAbility_Guard::GetStaminaCost() const {
  return UDrPlayerCombatData::Get()->Guard.StaminaCost;
}

void UDrGameplayAbility_Guard::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
  if (!CommitAbility(Handle, ActorInfo, ActivationInfo) || !CommitStamina()) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    return;
  }
  Elapsed = 0.f;
  GetDrASC()->SetLooseState(DrTags::State_Parrying, true);
  PlayAbilityMontage(UDrPlayerCombatData::Get()->Guard.Montage);
}

void UDrGameplayAbility_Guard::AbilityTick(float DeltaTime) {
  Elapsed += DeltaTime;
  UDrAbilitySystemComponent *ASC = GetDrASC();
  if (ASC && Elapsed >= UDrPlayerCombatData::Get()->Guard.ParryWindow &&
      ASC->HasMatchingGameplayTag(DrTags::State_Parrying)) {
    ASC->SetLooseState(DrTags::State_Parrying, false);
    ASC->SetLooseState(DrTags::State_Guarding, true);
  }
  if (Elapsed >= UDrPlayerCombatData::Get()->Guard.Duration)
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(),
               GetCurrentActivationInfo(), true, false);
}

void UDrGameplayAbility_Guard::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled) {
  if (UDrAbilitySystemComponent *ASC = GetDrASC()) {
    ASC->SetLooseState(DrTags::State_Parrying, false);
    ASC->SetLooseState(DrTags::State_Guarding, false);
  }
  if (bWasCancelled)
    StopAbilityMontage();
  Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility,
                    bWasCancelled);
}
