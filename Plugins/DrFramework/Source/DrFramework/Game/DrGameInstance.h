// DrFramework의 축소된 네트워크 부트 흐름을 관리하는 게임 인스턴스이다.
#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Network/DrNetTypes.h"
#include "TimerManager.h"
#include "DrGameInstance.generated.h"

UCLASS()
class DRFRAMEWORK_API UDrGameInstance : public UGameInstance {
  GENERATED_BODY()
public:
  virtual void Init() override;
  virtual void OnStart() override;
  virtual void Shutdown() override;
  // 원본 대응이 없는 축소 구현으로, 연결부터 초기 프로필 수신까지만 수행한다.
  void StartBootFlow();
  static UDrGameInstance *Get(const UObject *WorldContextObject);

private:
  void HandleConnectionStateChanged(EDrConnectionState NewState);
  void SendLoginRequest();
  void HandleLoginResponse(const FDrNetResponse &Response);
  void RequestInitialData();
  void HandleInitialProfileResponse(const FDrNetResponse &Response);
  void HandleSmokeTimeout();
  void ReportSmoke(const TCHAR *Step, bool bSucceeded);
  bool bLoginRequestInFlight = false;
  bool bInitialDataRequestInFlight = false;
  bool bBootCompleted = false;
  bool bAutoSmoke = false;
  FTimerHandle SmokeTimeoutHandle;
};
