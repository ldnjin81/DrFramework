#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "DrLightHitShake.generated.h"

/** 약공격 명중에 쓰는 짧고 약한 카메라 셰이크. */
UCLASS(Blueprintable)
class DRFRAMEWORK_API UDrLightHitShake : public UCameraShakeBase {
  GENERATED_BODY()
public:
  UDrLightHitShake(const FObjectInitializer& Initializer);
};
