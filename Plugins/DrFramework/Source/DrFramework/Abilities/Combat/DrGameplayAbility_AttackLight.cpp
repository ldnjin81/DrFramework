#include "Abilities/Combat/DrGameplayAbility_AttackLight.h"

#include "Core/DrGameplayTags.h"

UDrGameplayAbility_AttackLight::UDrGameplayAbility_AttackLight() {
  SetAssetTags(FGameplayTagContainer(DrTags::Ability_Attack_Light));
}

const TArray<FDrMeleeStage> &UDrGameplayAbility_AttackLight::GetStages() const {
  return UDrPlayerCombatData::Get()->LightStages;
}
