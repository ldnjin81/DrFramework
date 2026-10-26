// 어빌리티 시스템의 피해, 상태 효과, 자원 재생을 처리한다.
#include "Abilities/DrAbilitySystemComponent.h"
#include "Abilities/DrAttributeSet.h"
#include "Combat/DrPlayerCombatData.h"
#include "Abilities/DrGameplayAbility.h"
#include "Abilities/Effects/DrGameplayEffect_Damage.h"
#include "Core/DrGameplayTags.h"
#include "Engine/World.h"
#include "DrFramework.h"
#include "DrTableSubsystem.h"
#include "TableData/Generated/DrEffectDefsRow.h"
#include "TableData/Generated/EDrEffectApplyTo.h"

namespace {
FDrResolvedEffect ScaleForStack(const FDrResolvedEffect &Base, int32 Level) {
  FDrResolvedEffect Scaled = Base;
  if (Base.StackRule != EDrEffectStackRule::Accumulate ||
      !Base.StackBonus.IsValidIndex(Level - 1))
    return Scaled;
  const float Strength = 1.f + Base.StackBonus[Level - 1] * .01f;
  Scaled.DamagePerTick = Base.DamagePerTick * Strength;
  Scaled.Magnitude = Base.Magnitude * Strength;
  Scaled.ProcInstantDamageCurHpPct =
      Base.ProcInstantDamageCurHpPct * Strength;
  Scaled.ProcPoiseDamage = Base.ProcPoiseDamage * Strength;
  Scaled.MoveSpeedMultiplier =
      1.f + (Base.MoveSpeedMultiplier - 1.f) * Strength;
  Scaled.DamageTakenMultiplier =
      1.f + (Base.DamageTakenMultiplier - 1.f) * Strength;
  Scaled.DamageDealtMultiplier =
      1.f + (Base.DamageDealtMultiplier - 1.f) * Strength;
  return Scaled;
}
} // namespace

UDrAbilitySystemComponent::UDrAbilitySystemComponent() {
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;
}
void UDrAbilitySystemComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
  TickActiveAbilities(DeltaTime);
  // 효과 지속·주기 피해·쿨다운도 판정과 같은 틱 누적 시간으로 진행한다.
  TickTimedEffects(DeltaTime);
  TickCooldowns(DeltaTime);
  StaminaRegenDelayRemaining = FMath::Max(0.f, StaminaRegenDelayRemaining - DeltaTime);
  const UDrAttributeSet *Attributes = GetSet<UDrAttributeSet>();
  // 죽은 액터는 아무것도 재생하지 않는다.
  if (!Attributes || HasMatchingGameplayTag(DrTags::State_Dead))
    return;
  // 스태미나는 소모 후 지연 시간이 지나면 최대치를 향해 재생한다.
  if (StaminaRegenDelayRemaining <= 0.f &&
      Attributes->GetStamina() < Attributes->GetMaxStamina())
    SetNumericAttributeBase(
        UDrAttributeSet::GetStaminaAttribute(),
        FMath::Min(Attributes->GetMaxStamina(),
                   Attributes->GetStamina() +
                       Attributes->GetStaminaRegenRate() * DeltaTime));
  // 포이즈는 최대치를 향해 계속 재생한다.
  if (Attributes->GetPoise() < Attributes->GetMaxPoise())
    SetNumericAttributeBase(
        UDrAttributeSet::GetPoiseAttribute(),
        FMath::Min(Attributes->GetMaxPoise(),
                   Attributes->GetPoise() +
                       Attributes->GetPoiseRegenRate() * DeltaTime));
}
void UDrAbilitySystemComponent::ApplyDamage(UAbilitySystemComponent *TargetASC,
                                            float Damage, float PoiseDamage,
                                            AActor *Causer) {
  if (!TargetASC || Damage <= 0)
    return;
  // 공격자 측 "주는 피해" 배수를 적용한다.
  Damage *= GetOutgoingDamageMultiplier();
  FGameplayEffectContextHandle Context = MakeEffectContext();
  Context.AddSourceObject(Causer);
  if (AActor *InstigatorActor = GetOwnerActor())
    Context.AddInstigator(InstigatorActor,
                          Causer ? Causer : InstigatorActor);
  FGameplayEffectSpecHandle Spec =
      MakeOutgoingSpec(UDrGameplayEffect_Damage::StaticClass(), 1, Context);
  if (Spec.IsValid()) {
    Spec.Data->SetSetByCallerMagnitude(DrTags::Data_Damage, Damage);
    Spec.Data->SetSetByCallerMagnitude(DrTags::Data_PoiseDamage, PoiseDamage);
    ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
  }
}
void UDrAbilitySystemComponent::ApplyDirectDamage(float Damage,
                                                  AActor *Causer) {
  ApplyDamage(this, Damage, 0, Causer);
}
void UDrAbilitySystemComponent::ApplyDirectPoiseDamage(float Damage,
                                                       AActor *Causer) {
  if (Damage <= 0)
    return;
  FGameplayEffectContextHandle Context = MakeEffectContext();
  Context.AddSourceObject(Causer);
  FGameplayEffectSpecHandle Spec =
      MakeOutgoingSpec(UDrGameplayEffect_Damage::StaticClass(), 1, Context);
  if (Spec.IsValid()) {
    Spec.Data->SetSetByCallerMagnitude(DrTags::Data_Damage, 0);
    Spec.Data->SetSetByCallerMagnitude(DrTags::Data_PoiseDamage, Damage);
    ApplyGameplayEffectSpecToSelf(*Spec.Data);
  }
}
float UDrAbilitySystemComponent::AggregateMultiplier(
    float FDrResolvedEffect::*Field) const {
  float Product = 1;
  // 한시 효과를 먼저 곱하고 같은 태그의 영구 효과는 중복 계산하지 않는다.
  for (const TPair<FGameplayTag, FDrResolvedEffect> &Pair : TimedEffects)
    Product *= Pair.Value.*Field;
  for (const TPair<FGameplayTag, FDrResolvedEffect> &Pair : PermanentEffects)
    if (!TimedEffects.Contains(Pair.Key))
      Product *= Pair.Value.*Field;
  return Product;
}
float UDrAbilitySystemComponent::GetMoveSpeedMultiplier() const {
  return AggregateMultiplier(&FDrResolvedEffect::MoveSpeedMultiplier);
}
float UDrAbilitySystemComponent::GetIncomingDamageMultiplier() const {
  return AggregateMultiplier(&FDrResolvedEffect::DamageTakenMultiplier);
}
float UDrAbilitySystemComponent::GetOutgoingDamageMultiplier() const {
  return AggregateMultiplier(&FDrResolvedEffect::DamageDealtMultiplier);
}
int32 UDrAbilitySystemComponent::GetStackCount(
    const FGameplayTag &StateTag) const {
  const int32 *Count = StackCounts.Find(StateTag);
  return Count ? *Count : 0;
}
void UDrAbilitySystemComponent::AddPermanentEffect(
    const FDrResolvedEffect &Effect) {
  if (Effect.StateTag.IsValid()) {
    AddLooseGameplayTag(Effect.StateTag);
    PermanentEffects.Add(Effect.StateTag, Effect);
  }
}
void UDrAbilitySystemComponent::RemovePermanentEffect(
    const FGameplayTag &StateTag) {
  if (StateTag.IsValid()) {
    RemoveLooseGameplayTag(StateTag);
    PermanentEffects.Remove(StateTag);
  }
}
void UDrAbilitySystemComponent::RemoveTimedEffect(const FGameplayTag &Tag) {
  RemoveLooseGameplayTag(Tag);
  TimedEffects.Remove(Tag);
  BaseTimedEffects.Remove(Tag);
  StackCounts.Remove(Tag);
  EffectClocks.Remove(Tag);
  StatusTickCausers.Remove(Tag);
}
void UDrAbilitySystemComponent::TickTimedEffects(float DeltaTime) {
  if (DeltaTime <= 0.f || EffectClocks.IsEmpty())
    return;
  TArray<FGameplayTag> Tags;
  EffectClocks.GetKeys(Tags);
  for (const FGameplayTag &Tag : Tags) {
    FEffectClock *Clock = EffectClocks.Find(Tag);
    if (!Clock)
      continue;
    const FDrResolvedEffect *Effect = TimedEffects.Find(Tag);
    // 만료 시각까지의 주기 피해만 준다(만료와 같은 시각의 틱은 포함).
    if (Effect && Effect->DamagePerTick > 0 && Effect->Period > 0) {
      Clock->SinceTick += FMath::Min(DeltaTime, Clock->Remaining);
      while (Clock->SinceTick >= Effect->Period) {
        Clock->SinceTick -= Effect->Period;
        const TWeakObjectPtr<AActor> *Source = StatusTickCausers.Find(Tag);
        ApplyDirectDamage(Effect->DamagePerTick, Source ? Source->Get() : nullptr);
        Clock = EffectClocks.Find(Tag);
        Effect = TimedEffects.Find(Tag);
        if (!Clock || !Effect)
          break;
      }
    }
    if (Clock && (Clock->Remaining -= DeltaTime) <= 0.f)
      RemoveTimedEffect(Tag);
  }
}
void UDrAbilitySystemComponent::StartCooldown(const FGameplayTag &CooldownTag, float Duration) {
  if (!CooldownTag.IsValid() || Duration <= 0.f)
    return;
  SetLooseState(CooldownTag, true);
  CooldownRemaining.Add(CooldownTag, Duration);
}
float UDrAbilitySystemComponent::GetCooldownRemaining(const FGameplayTag &CooldownTag) const {
  const float *Remaining = CooldownRemaining.Find(CooldownTag);
  return Remaining ? FMath::Max(0.f, *Remaining) : 0.f;
}
void UDrAbilitySystemComponent::TickCooldowns(float DeltaTime) {
  for (auto It = CooldownRemaining.CreateIterator(); It; ++It) {
    It.Value() -= DeltaTime;
    if (It.Value() <= 0.f) {
      SetLooseState(It.Key(), false);
      It.RemoveCurrent();
    }
  }
}
void UDrAbilitySystemComponent::ApplyStatusEffect(
    const FDrResolvedEffect &Effect, AActor *Causer) {
  if (!Effect.StateTag.IsValid() || Effect.Duration <= 0)
    return;
  const FGameplayTag StateTag = Effect.StateTag;
  const int32 PreviousCount = GetStackCount(StateTag);
  const int32 MaxCount = FMath::Max(1, Effect.StackBonus.Num());
  const int32 Count = Effect.StackRule == EDrEffectStackRule::Accumulate
                          ? FMath::Min(PreviousCount + 1, MaxCount)
                          : 1;
  StackCounts.Add(StateTag, Count);
  BaseTimedEffects.Add(StateTag, Effect);
  const FDrResolvedEffect Scaled =
      ScaleForStack(BaseTimedEffects.FindChecked(StateTag), Count);
  // 보호막 재적용 시 흡수 풀을 효과 크기로 다시 채운다.
  if (StateTag == DrTags::State_Buff_Shield && Scaled.Magnitude > 0)
    ShieldPool = Scaled.Magnitude;
  // 한시 효과와 영구 효과를 별도 맵에 보관해 만료 간섭을 막는다.
  TimedEffects.Add(StateTag, Scaled);
  if (Scaled.ProcInstantDamageCurHpPct > 0)
    if (const UDrAttributeSet *Attributes = GetSet<UDrAttributeSet>())
      ApplyDirectDamage(Attributes->GetHealth() *
                            Scaled.ProcInstantDamageCurHpPct * .01f,
                        Causer);
  if (Scaled.ProcPoiseDamage > 0)
    ApplyDirectPoiseDamage(Scaled.ProcPoiseDamage, Causer);
  // 같은 한시 효과를 재적용하면 태그 수는 유지하고 남은 시간만 갱신한다. 주기 위상은 유지한다.
  if (!EffectClocks.Contains(StateTag))
    AddLooseGameplayTag(StateTag);
  EffectClocks.FindOrAdd(StateTag).Remaining = Scaled.Duration;
  StatusTickCausers.Add(StateTag, TWeakObjectPtr<AActor>(Causer));
}
bool UDrAbilitySystemComponent::ApplyEffectById(
    int32 EffectId, UDrAbilitySystemComponent *TargetASC, AActor *Causer,
    const UDrTableSubsystem *Tables) {
  const UDrTableSubsystem *TableData = Tables ? Tables : UDrTableSubsystem::Get();
  if (!TableData) {
    UE_LOG(LogDr, Warning,
           TEXT("[Effect] 테이블 저장소가 없어 효과를 적용할 수 없습니다: Id=%d"),
           EffectId);
    return false;
  }

  const FDrEffectDefsRow *Row =
      TableData->FindRowByKey<FDrEffectDefsRow>(EffectId);
  if (!Row) {
    UE_LOG(LogDr, Warning, TEXT("[Effect] 효과 Id를 찾지 못했습니다: %d"),
           EffectId);
    return false;
  }

  UDrAbilitySystemComponent *DestinationASC = nullptr;
  switch (Row->ApplyTo) {
  case EDrEffectApplyTo::Caster:
    DestinationASC = this;
    break;
  case EDrEffectApplyTo::Victim:
    if (!TargetASC) {
      UE_LOG(LogDr, Warning,
             TEXT("[Effect] 피격 대상 ASC가 없어 효과를 적용하지 않습니다: Id=%d"),
             EffectId);
      return false;
    }
    DestinationASC = TargetASC;
    break;
  default:
    UE_LOG(LogDr, Warning,
           TEXT("[Effect] 알 수 없는 ApplyTo 값입니다: Id=%d, ApplyTo=%d"),
           EffectId, static_cast<int32>(Row->ApplyTo));
    return false;
  }

  const FDrResolvedEffect Effect = DrEffects::Resolve(*Row);
  // ApplyStatusEffect가 조용히 거부하는 조건을 먼저 검사해 성공 여부를 보존한다.
  if (!DestinationASC->GetWorld() || !Effect.StateTag.IsValid() ||
      Effect.Duration <= 0) {
    UE_LOG(LogDr, Warning,
           TEXT("[Effect] 적용 조건을 충족하지 못했습니다: Id=%d, Tag=%s, Duration=%g"),
           EffectId, *Effect.StateTag.ToString(), Effect.Duration);
    return false;
  }

  DestinationASC->ApplyStatusEffect(Effect, Causer);
  return true;
}
float UDrAbilitySystemComponent::AbsorbWithShield(float Damage) {
  if (Damage <= 0 || ShieldPool <= 0)
    return Damage;
  const float AbsorbedDamage = FMath::Min(Damage, ShieldPool);
  ShieldPool -= AbsorbedDamage;
  if (ShieldPool <= 0) {
    // 풀이 소진되면 지속시간이 남아도 보호막을 즉시 해제한다.
    ShieldPool = 0;
    RemoveTimedEffect(DrTags::State_Buff_Shield);
  }
  return Damage - AbsorbedDamage;
}
bool UDrAbilitySystemComponent::TryActivateAbilityByTag(
    const FGameplayTag &AbilityTag) {
  FGameplayTagContainer AbilityTags(AbilityTag);
  return TryActivateAbilitiesByTag(AbilityTags);
}
void UDrAbilitySystemComponent::SetLooseState(const FGameplayTag &StateTag,
                                              bool bEnabled) {
  SetLooseGameplayTagCount(StateTag, bEnabled ? 1 : 0);
}
void UDrAbilitySystemComponent::SpendStamina(float Amount) {
  const UDrAttributeSet *Attributes = GetSet<UDrAttributeSet>();
  if (Attributes && Amount > 0) {
    SetNumericAttributeBase(UDrAttributeSet::GetStaminaAttribute(),
                            FMath::Max(0.f, Attributes->GetStamina() - Amount));
    StaminaRegenDelayRemaining = UDrPlayerCombatData::Get()->StaminaRegenDelayAfterSpend;
  }
}
float UDrAbilitySystemComponent::GetStamina() const {
  const UDrAttributeSet *Attributes = GetSet<UDrAttributeSet>();
  return Attributes ? Attributes->GetStamina() : 0;
}
void UDrAbilitySystemComponent::PushStaminaRegenDelay(float Seconds) {
  StaminaRegenDelayRemaining = FMath::Max(StaminaRegenDelayRemaining, Seconds);
}
void UDrAbilitySystemComponent::ApplyHeal(float Amount) {
  const UDrAttributeSet *Attributes = GetSet<UDrAttributeSet>();
  if (Attributes && Amount > 0)
    SetNumericAttributeBase(UDrAttributeSet::GetHealthAttribute(),
                            FMath::Min(Attributes->GetMaxHealth(),
                                       Attributes->GetHealth() + Amount));
}
void UDrAbilitySystemComponent::TakePoiseDamage(float Amount, AActor *Causer) {
  if (Amount > 0)
    ApplyDirectPoiseDamage(Amount, Causer);
}
void UDrAbilitySystemComponent::TickActiveAbilities(float DeltaTime) {
  // 틱 도중 어빌리티가 끝나거나 취소될 수 있으므로 인스턴스를 먼저 모은다.
  TArray<TWeakObjectPtr<UDrGameplayAbility>, TInlineAllocator<4>> Active;
  for (const FGameplayAbilitySpec &Spec : GetActivatableAbilities())
    if (Spec.IsActive())
      if (UDrGameplayAbility *Ability =
              Cast<UDrGameplayAbility>(Spec.GetPrimaryInstance()))
        Active.Add(Ability);
  for (const TWeakObjectPtr<UDrGameplayAbility> &Ability : Active)
    if (Ability.IsValid() && Ability->IsActive())
      Ability->AbilityTick(DeltaTime);
}
