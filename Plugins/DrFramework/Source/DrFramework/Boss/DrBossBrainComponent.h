#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TableData/Generated/DrBossPhaseRow.h"
#include "TableData/Generated/DrBossAttackRow.h"
#include "DrBossBrainComponent.generated.h"

class ADrBossCharacter;

UENUM(BlueprintType)
enum class EDrBossState : uint8 { Idle, Approach, Attack, Recover, Groggy, PhaseTransition, Dead };

/** 틱 누적 시간만으로 패턴·페이즈·반응을 진행한다. 애니와 타이머에 의존하지 않는다. */
UCLASS(ClassGroup=(Dr), meta=(BlueprintSpawnableComponent))
class DRFRAMEWORK_API UDrBossBrainComponent : public UActorComponent {
  GENERATED_BODY()
public:
  UDrBossBrainComponent();
  virtual void BeginPlay() override;
  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  void SetTarget(AActor* InTarget) { Target = InTarget; }
  void SetRandomSeed(int32 InSeed) { Random.Initialize(InSeed); }
  void InjectTables(const TArray<FDrBossPhaseRow>& InPhases, const TArray<FDrBossAttackRow>& InAttacks);
  EDrBossState GetState() const { return State; }
  int32 GetPhase() const { return Phases.IsValidIndex(PhaseIndex) ? Phases[PhaseIndex].Phase : 0; }
  const FDrBossAttackRow* GetCurrentAttack() const { return Attacks.IsValidIndex(AttackIndex) ? &Attacks[AttackIndex] : nullptr; }
  /** 선택은 쿨다운을 소비하지 않는다. 공격을 시작할 때만 소비한다. */
  int32 PickAttack(float Distance);
  void CheckPhaseThreshold();
  void EnterGroggy();
  void EnterDead();
  void ResetEncounter();
  void SetEngageAllowed(bool bInAllowed) { bEngageAllowed = bInAllowed; }
  bool IsEngageAllowed() const { return bEngageAllowed; }
private:
  void SetState(EDrBossState InState);
  void BeginAttack(int32 Index);
  void TurnToTarget(float Seconds);
  void StartMove(float Duration);
  void StopMove();
  void ApplyHits();
  bool HasLivingTarget() const;
  bool IsFacingTarget() const;
  void WarnMissingTables();
  ADrBossCharacter* Boss() const;
  UPROPERTY(Transient) TArray<FDrBossPhaseRow> Phases;
  UPROPERTY(Transient) TArray<FDrBossAttackRow> Attacks;
  UPROPERTY(Transient) EDrBossState State = EDrBossState::Idle;
  TWeakObjectPtr<AActor> Target;
  TSet<TWeakObjectPtr<AActor>> HitActors;
  TMap<int32, double> CooldownUntil;
  FRandomStream Random{38};
  int32 PhaseIndex = 0;
  int32 PendingPhase = INDEX_NONE;
  int32 AttackIndex = INDEX_NONE;
  double Clock = 0;
  double StateElapsed = 0;
  FVector LockedDirection = FVector::ForwardVector;
  bool bDirectionLocked = false;
  bool bInjected = false;
  bool bWarnedMissingTables = false;
  bool bEngageAllowed = true;
};
