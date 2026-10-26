#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

namespace DrGameAssets
{
  DRFRAMEWORK_API FSoftObjectPath Path(FName Key);

  template <typename T> T* Load(FName Key) {
    return Cast<T>(Path(Key).TryLoad());
  }
}
