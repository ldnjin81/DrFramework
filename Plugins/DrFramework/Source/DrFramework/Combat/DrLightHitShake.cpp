#include "Combat/DrLightHitShake.h"

#include "Combat/DrHitShakePattern.h"

UDrLightHitShake::UDrLightHitShake(const FObjectInitializer& Initializer) : Super(Initializer) {
  bSingleInstance = true;
  auto* Pattern = CreateDefaultSubobject<UPerlinNoiseCameraShakePattern>(TEXT("Perlin"));
  DrHitShake::Configure(Pattern, 1.f, .16f);
  SetRootShakePattern(Pattern);
}
