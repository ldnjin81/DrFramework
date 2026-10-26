#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DrHitFeedbackSubsystem.generated.h"

class UDrCombatFeelData;
class USkeletalMeshComponent;
class UMaterialInterface;
enum class EDrHitWeight : uint8 { Light, Heavy, Boss };

USTRUCT()
struct FDrHitFlashMesh {
  GENERATED_BODY()
  TWeakObjectPtr<USkeletalMeshComponent> Mesh;
  UPROPERTY() TObjectPtr<UMaterialInterface> Previous;
};
USTRUCT()
struct FDrHitFeedbackState {
  GENERATED_BODY()
  float StopRemaining = 0.f;
  float FlashRemaining = 0.f;
  float PreviousDilation = 1.f;
  bool bStopped = false;
  UPROPERTY() TArray<FDrHitFlashMesh> Meshes;
};

/** 액터의 CustomTimeDilation과 무관한 월드 시간으로 연출을 복구한다. */
UCLASS()
class DRFRAMEWORK_API UDrHitFeedbackSubsystem : public UTickableWorldSubsystem {
  GENERATED_BODY()
public:
  void PlayHit(AActor* Attacker, AActor* Victim, EDrHitWeight Weight);
  void PlaySlam(AActor* Source);
  void ResetActor(AActor* Actor);
  virtual void Tick(float DeltaTime) override;
  virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UDrHitFeedbackSubsystem, STATGROUP_Tickables); }
  virtual void Deinitialize() override;
protected:
  virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
public:
  UPROPERTY() TObjectPtr<UDrCombatFeelData> Settings;
private:
  const UDrCombatFeelData* Data();
  void Stop(AActor* Actor, float Duration);
  void Flash(AActor* Actor);
  void Shake(AActor* Actor, bool bHeavy, float Attenuation = 1.f);
  void RestoreFlash(FDrHitFeedbackState& State);
  UPROPERTY() TMap<TWeakObjectPtr<AActor>, FDrHitFeedbackState> Active;
};
