#include "Boss/DrBossData.h"

UDrBossData::UDrBossData() {
  DisplayName = NSLOCTEXT("DrBoss", "Boss", "보스");
  Stats.BaseMaxHealth = 2000.f;
  Stats.BaseMaxPoise = 100.f;
  Stats.PoiseRegenRate = 0.f;
}
