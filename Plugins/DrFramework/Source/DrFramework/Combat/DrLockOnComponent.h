#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DrLockOnComponent.generated.h"

class UDrLockOnTargetComponent;
class UDrCombatFeelData;
DECLARE_MULTICAST_DELEGATE_OneParam(FDrLockOnChangedDelegate, AActor*);

/** 획득·전환·해제와 카메라/스트레이프 상태 복구를 담당한다. */
UCLASS(ClassGroup=(Dr), meta=(BlueprintSpawnableComponent))
class DRFRAMEWORK_API UDrLockOnComponent : public UActorComponent {
  GENERATED_BODY()
public:
  UDrLockOnComponent();
  bool Toggle();
  void Clear();
  bool SwitchTarget(float Direction);
  void LookInput(float Axis);
  AActor* GetTarget() const;
  FVector GetTargetLocation() const;
  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
  virtual void EndPlay(const EEndPlayReason::Type Reason) override;
  FDrLockOnChangedDelegate OnTargetChanged;
  UPROPERTY(EditAnywhere) TObjectPtr<UDrCombatFeelData> Settings;
private:
  const UDrCombatFeelData* Data() const;
  bool IsVisible(UDrLockOnTargetComponent* Candidate) const;
  UDrLockOnTargetComponent* FindTarget(float Direction) const;
  void SetTarget(UDrLockOnTargetComponent* Candidate);
  UPROPERTY(Transient) TWeakObjectPtr<UDrLockOnTargetComponent> Target;
  bool bLocked = false;
  bool bSavedOrient = true;
  bool bSavedDesired = false;
  bool bSavedYaw = false;
  bool bSwitchLatched = false;
  float SavedArmLength = 300.f;
  FVector SavedSocketOffset = FVector::ZeroVector;
  bool bRestoreSocket = false;
  float OccludedFor = 0.f;
  float SwitchRemaining = 0.f;
};
