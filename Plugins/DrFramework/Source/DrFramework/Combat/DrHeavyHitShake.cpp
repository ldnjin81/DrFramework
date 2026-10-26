#include "Combat/DrHeavyHitShake.h"

#include "Combat/DrHitShakePattern.h"

UDrHeavyHitShake::UDrHeavyHitShake(const FObjectInitializer& Initializer) : Super(Initializer) {
  bSingleInstance = true;
  auto* Pattern = CreateDefaultSubobject<UPerlinNoiseCameraShakePattern>(TEXT("Perlin"));
  DrHitShake::Configure(Pattern, 2.5f, .3f);
  SetRootShakePattern(Pattern);
}
