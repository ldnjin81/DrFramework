#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DrEncounterSubsystem.generated.h"

class ADrCharacter;
class ADrBossCharacter;

UENUM(BlueprintType)
enum class EDrEncounterState : uint8 { Idle, Fighting, PlayerDead, Victory };

DECLARE_MULTICAST_DELEGATE_TwoParams(FDrEncounterStateChangedDelegate, EDrEncounterState, EDrEncounterState);
DECLARE_MULTICAST_DELEGATE(FDrEncounterActorsChangedDelegate);
DECLARE_MULTICAST_DELEGATE_TwoParams(FDrEncounterDoorMovingDelegate, AActor* /*Door*/, bool /*bMoving*/);

/** 월드 안에서만 체크포인트·문·사망·승리를 관리한다. */
UCLASS()
class DRFRAMEWORK_API UDrEncounterSubsystem : public UTickableWorldSubsystem {
  GENERATED_BODY()
public:
  virtual void OnWorldBeginPlay(UWorld &InWorld) override;
  virtual void Deinitialize() override;
  virtual void Tick(float DeltaTime) override;
  virtual TStatId GetStatId() const override;
  void BindActors(ADrCharacter *InPlayer, ADrBossCharacter *InBoss, AActor *InCheckpoint,
                  AActor *InDoor, AActor *InStart);
  void Advance(float DeltaSeconds);
  void SetDoorClosedZ(float InZ);
  EDrEncounterState GetState() const { return State; }
  bool IsCheckpointActive() const { return bCheckpointActive; }
  FTransform GetRespawnTransform() const;
  void RestartCheckpoint();
  bool IsBoundPlayer(const ADrCharacter *InPlayer) const;
  FDrEncounterStateChangedDelegate OnStateChanged;
  FDrEncounterActorsChangedDelegate OnActorsChanged;
  /** 아레나 문이 움직이기 시작하거나 멈출 때 알린다. */
  FDrEncounterDoorMovingDelegate OnDoorMoving;
  ADrBossCharacter *GetBoss() const;
  ADrCharacter *GetPlayer() const { return Player.Get(); }

  UPROPERTY(EditAnywhere, Category = "Dr|Encounter") float CheckpointRadius = 400.f;
  UPROPERTY(EditAnywhere, Category = "Dr|Encounter") float RespawnDelay = 3.f;
  UPROPERTY(EditAnywhere, Category = "Dr|Encounter") float DoorMoveDuration = 1.f;
  UPROPERTY(EditAnywhere, Category = "Dr|Encounter") float ArenaEnterDepth = 300.f;
protected:
  virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
private:
  void BindPlayer(ADrCharacter *InPlayer);
  void UnbindActors();
  void SetState(EDrEncounterState InState);
  void MoveDoor(bool bClose);
  void AdvanceDoor(float DeltaSeconds);
  void HandlePlayerDeath(AActor *InstigatorActor, AActor *Causer);
  void HandleBossDeath(AActor *InstigatorActor, AActor *Causer);
  UFUNCTION() void HandleActorEndPlay(AActor *Actor, EEndPlayReason::Type Reason);

  TWeakObjectPtr<ADrCharacter> Player;
  TWeakObjectPtr<ADrBossCharacter> Boss;
  TWeakObjectPtr<AActor> Checkpoint;
  TWeakObjectPtr<AActor> Door;
  TWeakObjectPtr<AActor> Start;
  EDrEncounterState State = EDrEncounterState::Idle;
  bool bCheckpointActive = false;
  bool bRespawnPending = false;
  float DeathElapsed = 0.f;
  float DoorElapsed = 0.f;
  FVector DoorOpen = FVector::ZeroVector;
  FVector DoorClosed = FVector::ZeroVector;
  FVector DoorFrom = FVector::ZeroVector;
  FVector DoorTo = FVector::ZeroVector;
  FVector ArenaDirection = FVector::ForwardVector;
  bool bDoorMoving = false;
  FTransform BossHome = FTransform::Identity;
};
