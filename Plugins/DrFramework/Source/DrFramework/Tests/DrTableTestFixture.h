// 기반 테이블(EffectDefs) 행과 인덱스를 메모리에 주입하는 격리 픽스처. 굽기 에셋과 게임 테이블에 의존하지 않는다.
// Design/Tables의 EffectDefs 원본을 바꾸면 Tools/gen_tables.ps1 뒤 이 스냅샷을 갱신한다.
#pragma once
#include "DrTableSubsystem.h"
#include "UObject/StrongObjectPtr.h"
#include "TableData/Generated/DrTableRegistration.h"
#include "TableData/Generated/DrEffectDefsTable.h"

namespace DrTableTests {
struct FFixture {
  TStrongObjectPtr<UDrTableSubsystem> Data{NewObject<UDrTableSubsystem>()};
  FFixture() {
    Data->AssetRoot.Empty();
    DrGeneratedTables::RegisterAll(*Data.Get());
    auto *Asset = NewObject<UDrEffectDefsTable>(Data.Get());
    Asset->SchemaHash = DrGeneratedTables::EffectDefsSchemaHash;
    auto AddRow = [Asset](int32 Id, EDrEffectCategory Category, EDrEffectApplyTo ApplyTo,
                          float Duration, float DamagePerTick, float Period, float Magnitude,
                          float MoveSpeed, float DamageDealt, int32 BonusStart,
                          EDrEffectStackRule StackRule, const TCHAR *Name) {
      FDrEffectDefsRow Row;
      Row.Id = Id;
      Row.Category = Category;
      Row.ApplyTo = ApplyTo;
      Row.Duration = Duration;
      Row.DamagePerTick = DamagePerTick;
      Row.Period = Period;
      Row.Magnitude = Magnitude;
      Row.MoveSpeedMultiplier = MoveSpeed;
      Row.DamageTakenMultiplier = 1.0f;
      Row.DamageDealtMultiplier = DamageDealt;
      Row.StackBonus_Start = BonusStart;
      Row.StackBonus_Num = 3;
      Row.DisplayName = FText::FromString(Name);
      Row.StackRule = StackRule;
      Asset->Rows.Add(Row);
    };
    AddRow(2001, EDrEffectCategory::Debuff, EDrEffectApplyTo::Victim, 3.f, 5.f, 1.f, 0.f, 1.f, 1.f, 0,
           EDrEffectStackRule::Accumulate, TEXT("화상 효과"));
    AddRow(2002, EDrEffectCategory::Debuff, EDrEffectApplyTo::Victim, 4.f, 0.f, 0.f, 0.f, .5f, 1.f, 3,
           EDrEffectStackRule::Accumulate, TEXT("빙결 효과"));
    AddRow(2003, EDrEffectCategory::Buff, EDrEffectApplyTo::Caster, 8.f, 0.f, 0.f, 120.f, 1.f, 1.25f, 6,
           EDrEffectStackRule::Refresh, TEXT("축복 효과"));
    Asset->PrimaryKeys = {2001, 2002, 2003};
    Asset->Category_Keys = {EDrEffectCategory::Buff, EDrEffectCategory::Debuff};
    Asset->Category_Offsets = {0, 1, 3};
    Asset->Category_Indices = {2, 0, 1};
    Asset->StackBonus_Pool = {0, 5, 10, 0, 3, 6, 0, 10, 20};
    Data->ExtraAssets.Add(TSoftObjectPtr<UPrimaryDataAsset>(Asset));
    Data->LoadAllTables();
    UDrTableSubsystem::SetOverride(Data.Get());
  }
  ~FFixture() { UDrTableSubsystem::ClearOverride(); }
};
} // namespace DrTableTests
