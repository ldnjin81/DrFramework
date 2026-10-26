// 모든 DrFramework 게임플레이 어빌리티의 공통 기반을 선언한다.
#pragma once
#include "Abilities/GameplayAbility.h"
#include "CoreMinimal.h"
#include "DrGameplayAbility.generated.h"
class UAnimMontage;
class UDrAbilitySystemComponent;
/**
 * 모든 DrFramework 어빌리티의 기반 클래스이다.
 * 스태미나 비용과 재사용 대기시간, 소유 액터 접근자를 제공한다.
 */
UCLASS(Abstract)
class DRFRAMEWORK_API UDrGameplayAbility : public UGameplayAbility {
  GENERATED_BODY()

public:
  UDrGameplayAbility();

  /** 어빌리티가 커밋될 때 소모되는 스태미나. 0이면 무료다. */
  UPROPERTY(EditDefaultsOnly, Category = "Dr|Ability")
  float StaminaCost = 0;

  /** 쿨다운 길이(초). 0이면 쿨다운이 없다. */
  UPROPERTY(EditDefaultsOnly, Category = "Dr|Cooldown")
  float CooldownDuration = 0;

  /** 쿨다운 동안 ASC에 적용되는 루즈 태그. */
  UPROPERTY(EditDefaultsOnly, Category = "Dr|Cooldown")
  FGameplayTag CooldownTag;

  UFUNCTION(BlueprintPure, Category = "Dr|Ability")
  AActor *GetDrActor() const;
  UFUNCTION(BlueprintPure, Category = "Dr|Ability")
  UDrAbilitySystemComponent *GetDrASC() const;
  /** 남은 재사용 대기시간을 반환하며 없으면 0이다. */
  UFUNCTION(BlueprintPure, Category = "Dr|Cooldown")
  float GetCooldownRemaining() const;
  /** 활성 중일 때 ASC가 매 틱 호출한다. 타이머 대신 이 값으로 구간을 잰다. */
  virtual void AbilityTick(float DeltaTime) {}
  virtual bool CanActivateAbility(
      const FGameplayAbilitySpecHandle Handle,
      const FGameplayAbilityActorInfo *ActorInfo,
      const FGameplayTagContainer *SourceTags = nullptr,
      const FGameplayTagContainer *TargetTags = nullptr,
      OUT FGameplayTagContainer *OptionalRelevantTags = nullptr) const override;

protected:
  /** 이번 발동의 기력 비용. 데이터 에셋에서 읽는 어빌리티가 재정의한다. */
  virtual float GetStaminaCost() const { return StaminaCost; }
  /** 소유자가 비용 이상의 스태미나를 가지고 있는지 확인한다. */
  bool HasEnoughStamina() const;
  /** 감당할 수 있을 때만 스태미나 비용을 소모한다. */
  bool CommitStamina();
  /** 재사용 대기 태그를 부여한다. ASC가 틱 누적으로 해제한다. */
  void CommitCooldown();
  /** 루트 모션 없는 동작을 일정 속도로 밀어 준다(공격 전진·대시). */
  void ApplyConstantMove(const FVector &Direction, float Distance,
                         float Duration, float EndSpeed);
  void ClearConstantMove();
  /** 아바타가 표시 메시를 가진 캐릭터일 때만 몽타주를 읽어 재생한다. 재생하지 못하면 nullptr이다. */
  UAnimMontage *PlayAbilityMontage(const TSoftObjectPtr<UAnimMontage> &Montage,
                          float PlayRate = 1.f);
  void StopAbilityMontage();
  /** 사망·경직·다른 전투 동작 중에는 발동하지 못하게 막는다. */
  void BlockDuringCombatStates();

private:
  UPROPERTY(Transient)
  TObjectPtr<UAnimMontage> PlayingMontage;
};
