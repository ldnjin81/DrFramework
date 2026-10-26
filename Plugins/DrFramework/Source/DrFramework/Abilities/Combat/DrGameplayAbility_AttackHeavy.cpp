#include "Abilities/Combat/DrGameplayAbility_AttackHeavy.h"

#include "Core/DrGameplayTags.h"

UDrGameplayAbility_AttackHeavy::UDrGameplayAbility_AttackHeavy() {
  SetAssetTags(FGameplayTagContainer(DrTags::Ability_Attack_Heavy));
}

const TArray<FDrMeleeStage> &UDrGameplayAbility_AttackHeavy::GetStages() const {
  return UDrPlayerCombatData::Get()->HeavyStages;
}
