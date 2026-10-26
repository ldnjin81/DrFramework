#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "DrSmokeSubsystem.generated.h"

/** 패키지에서 실제 메뉴·맵 전환을 실행하고 결과를 기록한다. */
UCLASS()
class DRFRAMEWORK_API UDrSmokeSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()
public:
  virtual void Initialize(FSubsystemCollectionBase& Collection) override;
  virtual void Deinitialize() override;
private:
  bool Tick(float DeltaSeconds);
  void Finish(bool bPassed);
  void Capture(const TCHAR* Name);
  FTSTicker::FDelegateHandle Ticker;
  TMap<FString, bool> Steps;
  int32 Stage = 0;
  double Started = 0;
  double StageStarted = 0;
  double FrameSeconds = 0;
  double LastFrame = 0;
  int32 Frames = 0;
};
