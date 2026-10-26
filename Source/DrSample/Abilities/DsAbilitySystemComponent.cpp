#include "Abilities/DsAbilitySystemComponent.h"
#include "Abilities/DsAttributeSet.h"
#include "Combat/DsPlayerCombatData.h"
#include "Abilities/Effects/DsGameplayEffect_Damage.h"
#include "Core/DsGameplayTags.h"
#include "TableData/Generated/DsEffectExtensionsRow.h"
void UDsAbilitySystemComponent::ApplyDamage(UAbilitySystemComponent *TargetASC,
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
      MakeOutgoingSpec(UDsGameplayEffect_Damage::StaticClass(), 1, Context);
  if (Spec.IsValid()) {
    Spec.Data->SetSetByCallerMagnitude(DsTags::Data_Damage, Damage);
    Spec.Data->SetSetByCallerMagnitude(DsTags::Data_PoiseDamage, PoiseDamage);
    ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
  }
}

void UDsAbilitySystemComponent::ApplyDirectDamage(float Damage,
                                                  AActor *Causer) {
  ApplyDamage(this, Damage, 0, Causer);
}

void UDsAbilitySystemComponent::ApplyDirectPoiseDamage(float Damage,
                                                       AActor *Causer) {
  if (Damage <= 0)
    return;
  FGameplayEffectContextHandle Context = MakeEffectContext();
  Context.AddSourceObject(Causer);
  FGameplayEffectSpecHandle Spec =
      MakeOutgoingSpec(UDsGameplayEffect_Damage::StaticClass(), 1, Context);
  if (Spec.IsValid()) {
    Spec.Data->SetSetByCallerMagnitude(DsTags::Data_Damage, 0);
    Spec.Data->SetSetByCallerMagnitude(DsTags::Data_PoiseDamage, Damage);
    ApplyGameplayEffectSpecToSelf(*Spec.Data);
  }
}

void UDsAbilitySystemComponent::SpendStamina(float Amount) {
  const UDsAttributeSet *Attributes = GetSet<UDsAttributeSet>();
  if (Attributes && Amount > 0) {
    SetNumericAttributeBase(UDsAttributeSet::GetStaminaAttribute(),
                            FMath::Max(0.f, Attributes->GetStamina() - Amount));
    StaminaRegenDelayRemaining = UDsPlayerCombatData::Get()->StaminaRegenDelayAfterSpend;
  }
}

float UDsAbilitySystemComponent::GetStamina() const {
  const UDsAttributeSet *Attributes = GetSet<UDsAttributeSet>();
  return Attributes ? Attributes->GetStamina() : 0;
}

void UDsAbilitySystemComponent::PushStaminaRegenDelay(float Seconds) {
  StaminaRegenDelayRemaining = FMath::Max(StaminaRegenDelayRemaining, Seconds);
}

void UDsAbilitySystemComponent::ApplyHeal(float Amount) {
  const UDsAttributeSet *Attributes = GetSet<UDsAttributeSet>();
  if (Attributes && Amount > 0)
    SetNumericAttributeBase(UDsAttributeSet::GetHealthAttribute(),
                            FMath::Min(Attributes->GetMaxHealth(),
                                       Attributes->GetHealth() + Amount));
}

void UDsAbilitySystemComponent::TakePoiseDamage(float Amount, AActor *Causer) {
  if (Amount > 0)
    ApplyDirectPoiseDamage(Amount, Causer);
}
void UDsAbilitySystemComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) {
 Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
  StaminaRegenDelayRemaining = FMath::Max(0.f, StaminaRegenDelayRemaining - DeltaTime);
  const UDsAttributeSet *Attributes = GetSet<UDsAttributeSet>();
  // 죽은 액터는 아무것도 재생하지 않는다.
  if (!Attributes || HasMatchingGameplayTag(DsTags::State_Dead))
    return;
  // 스태미나는 소모 후 지연 시간이 지나면 최대치를 향해 재생한다.
  if (StaminaRegenDelayRemaining <= 0.f &&
      Attributes->GetStamina() < Attributes->GetMaxStamina())
    SetNumericAttributeBase(
        UDsAttributeSet::GetStaminaAttribute(),
        FMath::Min(Attributes->GetMaxStamina(),
                   Attributes->GetStamina() +
                       Attributes->GetStaminaRegenRate() * DeltaTime));
  // 포이즈는 최대치를 향해 계속 재생한다.
  if (Attributes->GetPoise() < Attributes->GetMaxPoise())
    SetNumericAttributeBase(
        UDsAttributeSet::GetPoiseAttribute(),
        FMath::Min(Attributes->GetMaxPoise(),
                   Attributes->GetPoise() +
                       Attributes->GetPoiseRegenRate() * DeltaTime));
}
float UDsAbilitySystemComponent::GetCurrentHealth() const { const auto* A=GetSet<UDsAttributeSet>(); return A ? A->GetHealth() : 0; }
bool UDsAbilitySystemComponent::IsShieldEffect(const FDrResolvedEffect& Effect) const { return Effect.StateTag == DsTags::State_Buff_Shield; }

void UDsAbilitySystemComponent::OnEffectApplied(const FDrResolvedEffect& Effect,AActor* Causer) {
 if(const auto* Row=FDsEffectExtensionsRow::Find(Effect.DefinitionId)) {
  float Strength=1;
  const int32 Index=GetStackCount(Effect.StateTag)-1;
  if(Effect.StackRule==EDrEffectStackRule::Accumulate && Effect.StackBonus.IsValidIndex(Index)) Strength+=Effect.StackBonus[Index]*.01f;
  ApplyDirectPoiseDamage(Row->ProcPoiseDamage*Strength,Causer);
 }
}
