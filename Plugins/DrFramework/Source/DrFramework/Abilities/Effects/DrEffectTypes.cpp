// 구운 효과 정의를 런타임 효과 값으로 변환한다.
#include "Abilities/Effects/DrEffectTypes.h"
#include "TableData/Generated/DrEffectDefsRow.h"

FDrResolvedEffect DrEffects::Resolve(const FDrEffectDefsRow &Row) {
  FDrResolvedEffect Result;
  Result.StateTag = Row.StateTag;
  Result.Duration = Row.Duration;
  Result.DamagePerTick = Row.DamagePerTick;
  Result.Period = Row.Period;
  Result.Magnitude = Row.Magnitude;
  Result.MoveSpeedMultiplier = Row.MoveSpeedMultiplier;
  Result.DamageTakenMultiplier = Row.DamageTakenMultiplier;
  Result.DamageDealtMultiplier = Row.DamageDealtMultiplier;
  Result.ProcInstantDamageCurHpPct = Row.ProcInstantDamageCurHpPct;
  Result.ProcPoiseDamage = Row.ProcPoiseDamage;
  Result.StackRule = Row.StackRule;
  Result.StackBonus.Append(Row.GetStackBonus());
  return Result;
}
