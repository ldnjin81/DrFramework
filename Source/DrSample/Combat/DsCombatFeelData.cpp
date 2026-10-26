#include "Combat/DsCombatFeelData.h"
#include "TableData/DrGameAssets.h"

const UDsCombatFeelData* UDsCombatFeelData::Get() {
  static TWeakObjectPtr<UDsCombatFeelData> Cached;
  if (Cached.IsValid()) return Cached.Get();
  const auto* Data = LoadObject<UDsCombatFeelData>(nullptr, *DrGameAssets::Path(TEXT("DA_CombatFeel")).ToString());
  Cached = const_cast<UDsCombatFeelData*>(Data);
  return Data ? Data : GetDefault<UDsCombatFeelData>();
}
