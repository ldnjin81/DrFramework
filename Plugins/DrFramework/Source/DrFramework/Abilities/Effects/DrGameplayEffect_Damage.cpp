// 즉시형 피해 GameplayEffect의 메타 속성 변경자를 구성한다.
#include "Abilities/Effects/DrGameplayEffect_Damage.h"
#include "Abilities/DrAttributeSet.h"
#include "Core/DrGameplayTags.h"
UDrGameplayEffect_Damage::UDrGameplayEffect_Damage() {
  DurationPolicy = EGameplayEffectDurationType::Instant;
  FGameplayModifierInfo DamageModifier;
  // IncomingDamage += SetByCaller(Data.Damage)
  DamageModifier.Attribute = UDrAttributeSet::GetIncomingDamageAttribute();
  DamageModifier.ModifierOp = EGameplayModOp::Additive;
  FSetByCallerFloat DamageSetByCaller;
  DamageSetByCaller.DataTag = DrTags::Data_Damage;
  DamageModifier.ModifierMagnitude =
      FGameplayEffectModifierMagnitude(DamageSetByCaller);
  Modifiers.Add(DamageModifier);

  FGameplayModifierInfo PoiseDamageModifier;
  // IncomingPoiseDamage += SetByCaller(Data.PoiseDamage)
  PoiseDamageModifier.Attribute =
      UDrAttributeSet::GetIncomingPoiseDamageAttribute();
  PoiseDamageModifier.ModifierOp = EGameplayModOp::Additive;
  FSetByCallerFloat PoiseDamageSetByCaller;
  PoiseDamageSetByCaller.DataTag = DrTags::Data_PoiseDamage;
  PoiseDamageModifier.ModifierMagnitude =
      FGameplayEffectModifierMagnitude(PoiseDamageSetByCaller);
  Modifiers.Add(PoiseDamageModifier);
}
