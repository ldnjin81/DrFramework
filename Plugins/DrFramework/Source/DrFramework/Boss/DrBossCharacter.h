#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "DrBossCharacter.generated.h"

class UDrAbilitySystemComponent;
class UDrAttributeSet;
class UDrBossBrainComponent;
class UDrCombatFXComponent;
class UDrLockOnTargetComponent;
class UAnimSequence;
class UDrBossData;

/** 플레이어와 같은 GAS 피해 경로를 사용하는 Boss 보스이다. */
UCLASS()
class DRFRAMEWORK_API ADrBossCharacter : public ACharacter, public IAbilitySystemInterface {
  GENERATED_BODY()
public:
  ADrBossCharacter();
  virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
  virtual void PostInitializeComponents() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  UDrAbilitySystemComponent* GetDrASC() const { return AbilitySystem; }
  UDrAttributeSet* GetAttributes() const { return Attributes; }
  UDrBossBrainComponent* GetBrain() const { return Brain; }
  void PlaySequence(FName InName, bool bLoop);
  virtual void BeginPlay() override;
  void ResetBoss(const FTransform &Home);
  const FTransform &GetHomeTransform() const { return HomeTransform; }

  /** 이 보스의 데이터 에셋. 비어 있거나 못 불러오면 클래스 기본값을 쓴다. */
  UPROPERTY(EditAnywhere, Category="Dr|Boss") TSoftObjectPtr<UDrBossData> BossData;
  /** GameAssets에서 조회할 게임별 보스 데이터 키. */
  UPROPERTY(EditDefaultsOnly, Category="Dr|Boss") FName BossAssetKey = TEXT("Boss");
  const UDrBossData* GetData() const;
private:
  UPROPERTY(VisibleAnywhere) TObjectPtr<UDrCombatFXComponent> CombatFX;
  UPROPERTY(VisibleAnywhere) TObjectPtr<UDrLockOnTargetComponent> LockOnTarget;
  void HandlePoiseBroken(AActor* InstigatorActor, AActor* Causer);
  void HandleOutOfHealth(AActor* InstigatorActor, AActor* Causer);
  void HandleDamaged(float Damage, AActor* InstigatorActor, AActor* Causer);
  UPROPERTY(VisibleAnywhere, Category="Dr|Boss") TObjectPtr<UDrAbilitySystemComponent> AbilitySystem;
  UPROPERTY(VisibleAnywhere, Category="Dr|Boss") TObjectPtr<UDrAttributeSet> Attributes;
  UPROPERTY(VisibleAnywhere, Category="Dr|Boss") TObjectPtr<UDrBossBrainComponent> Brain;
  UPROPERTY(Transient) TMap<FName, TObjectPtr<UAnimSequence>> Animations;
  TSet<FName> MissingAnimations;
  bool bEventsBound = false;
  UPROPERTY(Transient) mutable TObjectPtr<const UDrBossData> LoadedData;
  FTransform HomeTransform = FTransform::Identity;
};
