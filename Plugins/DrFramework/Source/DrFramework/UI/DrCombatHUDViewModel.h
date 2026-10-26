#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Encounter/DrEncounterSubsystem.h"
#include "DrCombatHUDViewModel.generated.h"

class ADrCharacter;
class UAbilitySystemComponent;

/** GAS와 엔카운터 이벤트만 구독하는 전투 표시 모델이다. */
UCLASS(BlueprintType)
class DRFRAMEWORK_API UDrCombatHUDViewModel : public UMVVMViewModelBase {
  GENERATED_BODY()
public:
  void Initialize(UDrEncounterSubsystem *InEncounter);
  void BindPlayer(ADrCharacter *InPlayer);
  void Shutdown();
  virtual void BeginDestroy() override;

  UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Dr|HUD") float PlayerHealth = 0.f;
  UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Dr|HUD") float PlayerMaxHealth = 0.f;
  UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Dr|HUD") float PlayerStamina = 0.f;
  UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Dr|HUD") float PlayerMaxStamina = 0.f;
  UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Dr|HUD") bool BossVisible = false;
  UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Dr|HUD") FText BossDisplayName;
  UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Dr|HUD") float BossHealthRate = 0.f;
  UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Dr|HUD") EDrEncounterState EncounterState = EDrEncounterState::Idle;
  UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Dr|HUD") TObjectPtr<AActor> LockOnTarget;

  UFUNCTION(BlueprintPure, FieldNotify, Category="Dr|HUD") float GetPlayerHealthRate() const;
  UFUNCTION(BlueprintPure, FieldNotify, Category="Dr|HUD") float GetPlayerStaminaRate() const;
  UFUNCTION(BlueprintPure, FieldNotify, Category="Dr|HUD") bool IsPlayerDead() const;
  UFUNCTION(BlueprintPure, FieldNotify, Category="Dr|HUD") bool IsVictory() const;
private:
  void HandleLockOnChanged(AActor* Target);
  TWeakObjectPtr<ADrCharacter> Player;
  void RefreshPlayer();
  void RefreshBoss();
  void RefreshEncounter();
  void HandleStateChanged(EDrEncounterState OldState, EDrEncounterState NewState);
  void UnbindAttributes(UAbilitySystemComponent *ASC);
  TWeakObjectPtr<UAbilitySystemComponent> PlayerASC;
  TWeakObjectPtr<UAbilitySystemComponent> BossASC;
  TWeakObjectPtr<UDrEncounterSubsystem> Encounter;
};
