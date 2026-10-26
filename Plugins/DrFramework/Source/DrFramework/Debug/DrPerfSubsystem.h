#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "DrPerfSubsystem.generated.h"

class ACameraActor;

/** 패키지의 고정 60초 카메라 경로를 같은 조건으로 측정한다. */
UCLASS()
class DRFRAMEWORK_API UDrPerfSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()
public:
  virtual void Initialize(FSubsystemCollectionBase& Collection) override;
  virtual void Deinitialize() override;
private:
  bool Tick(float DeltaSeconds);
  bool Finish(const FString& Error = FString());
  struct FSample {
    double Frame = 0;
    float Game = 0, GPU = 0;
  };
  TArray<FSample> Samples[4];
  FTSTicker::FDelegateHandle Ticker;
  TWeakObjectPtr<ACameraActor> Camera;
  double Started = 0, CaptureStarted = 0, LastFrame = 0, WarmupStarted = 0;
  FVector ArenaCenter = FVector::ZeroVector;
  bool bTravelStarted = false;
};
