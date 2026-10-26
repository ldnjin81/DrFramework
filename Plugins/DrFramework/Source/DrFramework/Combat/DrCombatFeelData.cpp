#include "Combat/DrCombatFeelData.h"
#include "TableData/DrGameAssets.h"

const UDrCombatFeelData* UDrCombatFeelData::Get() {
  static TWeakObjectPtr<UDrCombatFeelData> Cached;
  if (Cached.IsValid()) return Cached.Get();
  const auto* Data = LoadObject<UDrCombatFeelData>(nullptr, *DrGameAssets::Path(TEXT("DA_CombatFeel")).ToString());
  Cached = const_cast<UDrCombatFeelData*>(Data);
  return Data ? Data : GetDefault<UDrCombatFeelData>();
}
