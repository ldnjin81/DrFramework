// 플러그인 자동화 테스트용 경량 월드와 어빌리티 소유 액터를 제공한다. 게임 데이터에 의존하지 않는다.
#pragma once
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Abilities/DrAbilitySystemComponent.h"

namespace DrTestWorld {
class FWorldFixture {
public:
  FWorldFixture() {
    UWorld::InitializationValues Values;
    Values.AllowAudioPlayback(false)
        .CreatePhysicsScene(false)
        .CreateNavigation(false)
        .CreateAISystem(false)
        .RequiresHitProxies(false)
        .ShouldSimulatePhysics(false)
        .SetTransactional(false);
    static int32 Serial = 0;
    // 각 픽스처가 고유한 월드 이름을 사용하게 한다.
    World = UWorld::CreateWorld(
        EWorldType::Game, false,
        FName(*FString::Printf(TEXT("DrFrameworkTestWorld_%d"), Serial++)),
        nullptr, true, ERHIFeatureLevel::Num, &Values);
  }
  ~FWorldFixture() {
    if (World)
      World->DestroyWorld(false);
  }
  UWorld *World = nullptr;
};

/** 속성 세트 없이 기반 ASC만 붙인 액터. 효과 배수·누적·만료·쿨다운은 기반만으로 검증한다. */
struct FAbilityOwner {
  explicit FAbilityOwner(UWorld *World) {
    Actor = World->SpawnActor<AActor>();
    ASC = NewObject<UDrAbilitySystemComponent>(Actor);
    Actor->AddInstanceComponent(ASC);
    ASC->RegisterComponent();
    ASC->InitAbilityActorInfo(Actor, Actor);
  }
  AActor *Actor = nullptr;
  UDrAbilitySystemComponent *ASC = nullptr;
};
} // namespace DrTestWorld
