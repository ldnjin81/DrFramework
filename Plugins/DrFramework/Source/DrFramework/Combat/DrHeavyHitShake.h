#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "DrHeavyHitShake.generated.h"

/** 강공격 명중에 쓰는 길고 강한 카메라 셰이크. */
UCLASS(Blueprintable)
class DRFRAMEWORK_API UDrHeavyHitShake : public UCameraShakeBase {
  GENERATED_BODY()
public:
  UDrHeavyHitShake(const FObjectInitializer& Initializer);
};
