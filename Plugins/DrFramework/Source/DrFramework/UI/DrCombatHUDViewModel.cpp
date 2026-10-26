#include "UI/DrCombatHUDViewModel.h"

#include "Abilities/DrAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Boss/DrBossCharacter.h"
#include "Boss/DrBossData.h"
#include "Characters/DrCharacter.h"
#include "Combat/DrLockOnComponent.h"

namespace {
float Rate(float Value, float Maximum) {
  return Maximum > 0.f ? FMath::Clamp(Value / Maximum, 0.f, 1.f) : 0.f;
}
}

float UDrCombatHUDViewModel::GetPlayerHealthRate() const { return Rate(PlayerHealth, PlayerMaxHealth); }
float UDrCombatHUDViewModel::GetPlayerStaminaRate() const { return Rate(PlayerStamina, PlayerMaxStamina); }
bool UDrCombatHUDViewModel::IsPlayerDead() const { return EncounterState == EDrEncounterState::PlayerDead; }
bool UDrCombatHUDViewModel::IsVictory() const { return EncounterState == EDrEncounterState::Victory; }

void UDrCombatHUDViewModel::Initialize(UDrEncounterSubsystem *InEncounter) {
  if (Encounter.IsValid()) {
    Encounter->OnActorsChanged.RemoveAll(this);
    Encounter->OnStateChanged.RemoveAll(this);
  }
  Encounter = InEncounter;
  if (InEncounter) {
    InEncounter->OnActorsChanged.AddUObject(this, &ThisClass::RefreshEncounter);
    InEncounter->OnStateChanged.AddUObject(this, &ThisClass::HandleStateChanged);
  }
  RefreshEncounter();
}

void UDrCombatHUDViewModel::UnbindAttributes(UAbilitySystemComponent *ASC) {
  if (!ASC) return;
  for (const auto Attribute : {UDrAttributeSet::GetHealthAttribute(), UDrAttributeSet::GetMaxHealthAttribute(),
                               UDrAttributeSet::GetStaminaAttribute(), UDrAttributeSet::GetMaxStaminaAttribute()})
    ASC->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
}

void UDrCombatHUDViewModel::BindPlayer(ADrCharacter *InPlayer) {
  if (Player.IsValid()) Player->GetLockOn()->OnTargetChanged.RemoveAll(this);
  Player = InPlayer;
  if (InPlayer) InPlayer->GetLockOn()->OnTargetChanged.AddUObject(this, &ThisClass::HandleLockOnChanged);
  HandleLockOnChanged(InPlayer ? InPlayer->GetLockOn()->GetTarget() : nullptr);
  UnbindAttributes(PlayerASC.Get());
  PlayerASC = InPlayer ? InPlayer->GetAbilitySystemComponent() : nullptr;
  if (auto *ASC = PlayerASC.Get()) {
    for (const auto Attribute : {UDrAttributeSet::GetHealthAttribute(), UDrAttributeSet::GetMaxHealthAttribute(),
                                 UDrAttributeSet::GetStaminaAttribute(), UDrAttributeSet::GetMaxStaminaAttribute()})
      ASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddWeakLambda(this,
        [this](const FOnAttributeChangeData &) { RefreshPlayer(); });
  }
  RefreshPlayer();
}

void UDrCombatHUDViewModel::RefreshPlayer() {
  auto *ASC = PlayerASC.Get();
  const auto Read = [ASC](FGameplayAttribute Attribute) { return ASC ? ASC->GetNumericAttribute(Attribute) : 0.f; };
  // 먼저 네 값을 확정한 뒤 파생 비율을 알린다. 최대치와 현재치가 함께 바뀌어도 일관된다.
  const bool HealthChanged = UE_MVVM_SET_PROPERTY_VALUE(PlayerHealth, Read(UDrAttributeSet::GetHealthAttribute()));
  const bool MaxHealthChanged = UE_MVVM_SET_PROPERTY_VALUE(PlayerMaxHealth, Read(UDrAttributeSet::GetMaxHealthAttribute()));
  const bool StaminaChanged = UE_MVVM_SET_PROPERTY_VALUE(PlayerStamina, Read(UDrAttributeSet::GetStaminaAttribute()));
  const bool MaxStaminaChanged = UE_MVVM_SET_PROPERTY_VALUE(PlayerMaxStamina, Read(UDrAttributeSet::GetMaxStaminaAttribute()));
  if (HealthChanged || MaxHealthChanged) UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetPlayerHealthRate);
  if (StaminaChanged || MaxStaminaChanged) UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetPlayerStaminaRate);
}

void UDrCombatHUDViewModel::RefreshEncounter() {
  UnbindAttributes(BossASC.Get());
  ADrBossCharacter *Boss = Encounter.IsValid() ? Encounter->GetBoss() : nullptr;
  BossASC = Boss ? Boss->GetAbilitySystemComponent() : nullptr;
  UE_MVVM_SET_PROPERTY_VALUE(BossDisplayName, Boss ? Boss->GetData()->DisplayName : FText::GetEmpty());
  if (auto *ASC = BossASC.Get()) {
    for (const auto Attribute : {UDrAttributeSet::GetHealthAttribute(), UDrAttributeSet::GetMaxHealthAttribute()})
      ASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddWeakLambda(this,
        [this](const FOnAttributeChangeData &) { RefreshBoss(); });
  }
  RefreshBoss();
  HandleStateChanged(EncounterState, Encounter.IsValid() ? Encounter->GetState() : EDrEncounterState::Idle);
}

void UDrCombatHUDViewModel::RefreshBoss() {
  auto *ASC = BossASC.Get();
  UE_MVVM_SET_PROPERTY_VALUE(BossHealthRate, ASC ? Rate(ASC->GetNumericAttribute(UDrAttributeSet::GetHealthAttribute()),
    ASC->GetNumericAttribute(UDrAttributeSet::GetMaxHealthAttribute())) : 0.f);
}

void UDrCombatHUDViewModel::HandleStateChanged(EDrEncounterState, EDrEncounterState NewState) {
  if (UE_MVVM_SET_PROPERTY_VALUE(EncounterState, NewState)) {
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsPlayerDead);
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsVictory);
  }
  UE_MVVM_SET_PROPERTY_VALUE(BossVisible, NewState == EDrEncounterState::Fighting && BossASC.IsValid());
}

void UDrCombatHUDViewModel::Shutdown() {
  if (Player.IsValid()) Player->GetLockOn()->OnTargetChanged.RemoveAll(this);
  Player.Reset();
  HandleLockOnChanged(nullptr);
  if (Encounter.IsValid()) {
    Encounter->OnActorsChanged.RemoveAll(this);
    Encounter->OnStateChanged.RemoveAll(this);
  }
  UnbindAttributes(PlayerASC.Get());
  UnbindAttributes(BossASC.Get());
  PlayerASC.Reset();
  BossASC.Reset();
  Encounter.Reset();
}

void UDrCombatHUDViewModel::HandleLockOnChanged(AActor* Target) {
  UE_MVVM_SET_PROPERTY_VALUE(LockOnTarget, Target);
}

void UDrCombatHUDViewModel::BeginDestroy() {
  Shutdown();
  Super::BeginDestroy();
}
