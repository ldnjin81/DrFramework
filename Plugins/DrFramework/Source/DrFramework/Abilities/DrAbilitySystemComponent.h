// DrFramework 전투 어빌리티 시스템 컴포넌트를 선언한다.
#pragma once
#include "AbilitySystemComponent.h"
#include "CoreMinimal.h"
#include "Abilities/Effects/DrEffectTypes.h"
#include "DrAbilitySystemComponent.generated.h"

class UDrTableSubsystem;

/**
 * 데미지 적용, 태그 기반 발동, 루즈 상태 처리와
 * 틱 기반 스태미나·포이즈 재생을 한곳에서 담당한다.
 */
UCLASS(ClassGroup = (Dr), meta = (BlueprintSpawnableComponent))
class DRFRAMEWORK_API UDrAbilitySystemComponent : public UAbilitySystemComponent {
  GENERATED_BODY()
  friend class FDrEffectSystemTestAccess;

public:
  UDrAbilitySystemComponent();
  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction *ThisTickFunction) override;
  /**
   * 기반 클래스는 틱이 필요한 게임플레이 태스크가 없으면 컴포넌트 틱을 끈다.
   * 스태미나·포이즈 재생과 어빌리티 시간 진행이 틱에 달려 있으므로 항상 켠다.
   */
  virtual bool GetShouldTick() const override { return true; }
  /** 타깃 ASC에 데미지 GameplayEffect를 적용한다. */
  void ApplyDamage(UAbilitySystemComponent *TargetASC, float Damage,
                   float PoiseDamage, AActor *Causer);
  /** 상태 효과를 일정 시간 부여하고 DoT면 주기 데미지를 적용한다. */
  void ApplyStatusEffect(const FDrResolvedEffect &Effect, AActor *Causer);
  /** 효과 정의 Id를 조회해 적용한다. Tables가 없으면 게임 전역 저장소를 쓴다. */
  bool ApplyEffectById(int32 EffectId, UDrAbilitySystemComponent *TargetASC,
                       AActor *Causer, const UDrTableSubsystem *Tables = nullptr);
  /** 장착 중 상시 효과를 타이머 없이 부여한다. */
  void AddPermanentEffect(const FDrResolvedEffect &Effect);
  /** 장착 중인 상시 효과를 해제한다. */
  void RemovePermanentEffect(const FGameplayTag &StateTag);
  /** 활성 효과들의 이동 속도 배수 곱이다. 효과가 없으면 1이다. */
  float GetMoveSpeedMultiplier() const;
  /** 활성 효과들의 받는 피해 배수 곱이다. 효과가 없으면 1이다. */
  float GetIncomingDamageMultiplier() const;
  /** 활성 효과들의 주는 피해 배수 곱이다. 효과가 없으면 1이다. */
  float GetOutgoingDamageMultiplier() const;
  /** 한시 효과의 현재 단계를 반환하며 없으면 0이다. */
  int32 GetStackCount(const FGameplayTag &StateTag) const;
  /** 보호막 흡수 풀을 차감하고 잔여 데미지를 반환한다. */
  float AbsorbWithShield(float Damage);
  /** 어빌리티 태그가 일치하는 부여된 어빌리티를 발동한다. */
  bool TryActivateAbilityByTag(const FGameplayTag &AbilityTag);
  /** 루즈 게임플레이 태그를 토글한다. */
  void SetLooseState(const FGameplayTag &StateTag, bool bEnabled);
  /** 스태미나를 소모하고 재생을 잠시 멈춘다. */
  void SpendStamina(float Amount);
  /** 현재 스태미나를 반환하며 속성 세트가 없으면 0이다. */
  float GetStamina() const;
  /** 쿨다운 태그를 켜고 지정 시간 뒤 틱에서 끈다. */
  void StartCooldown(const FGameplayTag &CooldownTag, float Duration);
  /** 남은 쿨다운 시간이며 없으면 0이다. */
  float GetCooldownRemaining(const FGameplayTag &CooldownTag) const;
  /** 스태미나 재생 억제 시간을 최소 지정 시간 이상으로 연장한다. */
  void PushStaminaRegenDelay(float Seconds);
  /** 최대 생명력을 넘지 않도록 지정량을 회복한다. */
  void ApplyHeal(float Amount);
  /** 생명력 피해 없이 포이즈만 깎는다(쳐내기 반격 등). */
  void TakePoiseDamage(float Amount, AActor *Causer);


protected:

private:
  /** 활성 중인 DrFramework 어빌리티의 시간 진행을 구동한다. */
  void TickActiveAbilities(float DeltaTime);
  /** 한시 효과의 남은 시간과 주기 피해를 틱 누적으로 진행한다. */
  void TickTimedEffects(float DeltaTime);
  void TickCooldowns(float DeltaTime);
  /** 자신에게 데미지 효과를 적용한다. */
  void ApplyDirectDamage(float Damage, AActor *Causer);
  /** 자신에게 포이즈 피해 효과를 적용한다. */
  void ApplyDirectPoiseDamage(float Damage, AActor *Causer);
  /** 활성 효과의 지정 필드를 곱으로 합산한다. 한시 효과가 우선한다. */
  float AggregateMultiplier(float FDrResolvedEffect::*Field) const;
  /** 한시 효과를 해제하며 같은 태그의 영구 효과는 건드리지 않는다. */
  void RemoveTimedEffect(const FGameplayTag &Tag);
  /** 타이머로 만료되는 효과이다. */
  TMap<FGameplayTag, FDrResolvedEffect> TimedEffects;
  /** 단계 계산의 기준이 되는 원본 효과이다. */
  TMap<FGameplayTag, FDrResolvedEffect> BaseTimedEffects;
  TMap<FGameplayTag, int32> StackCounts;
  /** 장착 동안 유지되는 효과이다. */
  TMap<FGameplayTag, FDrResolvedEffect> PermanentEffects;
  /** 상태별 해제 및 주기 처리 타이머이다. */
  /** 한시 효과의 남은 시간과 주기 위상(재적용해도 위상은 유지한다). */
  struct FEffectClock {
    float Remaining = 0.f;
    float SinceTick = 0.f;
  };
  TMap<FGameplayTag, FEffectClock> EffectClocks;
  TMap<FGameplayTag, float> CooldownRemaining;
  /** 주기 피해에 사용할 최근 적용자이다. */
  TMap<FGameplayTag, TWeakObjectPtr<AActor>> StatusTickCausers;
  /** 이 시간이 남아 있는 동안 스태미나 재생이 억제된다. */
  float StaminaRegenDelayRemaining = 0.f;
  /** 보호막이 흡수할 수 있는 남은 피해량이다. */
  float ShieldPool = 0.f;
};
