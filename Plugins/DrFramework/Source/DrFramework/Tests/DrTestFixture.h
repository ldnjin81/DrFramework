// 자동화 테스트용 경량 월드와 타이머 보조 함수를 제공한다.
#pragma once
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "TimerManager.h"
namespace DrTestFixture {
class FWorldFixture {
public:
  explicit FWorldFixture(bool bCreatePhysicsScene = false) {
    UWorld::InitializationValues Values;
    Values.AllowAudioPlayback(false)
        .CreatePhysicsScene(bCreatePhysicsScene)
        .CreateNavigation(false)
        .CreateAISystem(false)
        .RequiresHitProxies(false)
        .ShouldSimulatePhysics(false)
        .SetTransactional(false);
    static int32 Serial = 0;
    // 각 픽스처가 고유한 월드 이름을 사용하게 한다.
    World = UWorld::CreateWorld(
        EWorldType::Game, false,
        FName(*FString::Printf(TEXT("DrAutomationWorld_%d"), Serial++)),
        nullptr, true, ERHIFeatureLevel::Num, &Values);
  }
  ~FWorldFixture() {
    if (World)
      World->DestroyWorld(false);
  }
  UWorld *World = nullptr;
};
inline void WarmUpTimers(UWorld &World) {
  // 헤드리스 환경에서는 프레임과 타이머 매니저를 직접 한 번 진행시킨다.
  ++GFrameCounter;
  World.GetTimerManager().Tick(0.f);
}
} // namespace DrTestFixture
