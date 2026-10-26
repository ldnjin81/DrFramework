// 샘플 JSON과 같은 행·인덱스를 메모리에 주입하는 격리 픽스처.
// 원본 데이터 변경 시 Tools/gen_tables.ps1 뒤 이 스냅샷을 갱신한다.
#pragma once
#include "DrTableSubsystem.h"
#include "UObject/StrongObjectPtr.h"
#include "TableData/Generated/DrTableRegistration.h"
#include "TableData/Generated/DrBossAttackTable.h"
#include "TableData/Generated/DrBossPhaseTable.h"
#include "TableData/Generated/DrEffectDefsTable.h"
namespace DrTableTests {
struct FFixture {
 TStrongObjectPtr<UDrTableSubsystem> Data{NewObject<UDrTableSubsystem>()};
 FFixture() {
  Data->AssetRoot.Empty();
  DrGeneratedTables::RegisterAll(*Data.Get());
  {
   auto* Asset = NewObject<UDrBossAttackTable>(Data.Get());
   Asset->SchemaHash = TEXT("sha256:b1167b7fed433311cb8de93d40ed9898f4d909afaa7295207a3a41c8b2ca6ec4");
   {
    FDrBossAttackRow Row;
    Row.Id = 101;
    Row.PatternSet = FName(TEXT("Boss_Phase1"));
    Row.Kind = EDrBossAttackKind::Swing;
    Row.Anim = FName(TEXT("Swing1_Medium"));
    Row.Weight = 30;
    Row.Cooldown = FTimespan::FromSeconds(2000);
    Row.Damage = 18;
    Row.StaminaDamage = 8;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(350);
    Row.Duration = FTimespan::FromSeconds(1130);
    Row.ActiveStart = FTimespan::FromSeconds(500);
    Row.ActiveEnd = FTimespan::FromSeconds(780);
    Row.PoiseDamage = 20;
    Row.HitRadius = 180.0f;
    Row.HitForwardOffset = 130.0f;
    Row.MinRange = 0.0f;
    Row.MaxRange = 320.0f;
    Row.MoveDistance = 0.0f;
    Row.Heal = 0;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossAttackRow Row;
    Row.Id = 102;
    Row.PatternSet = FName(TEXT("Boss_Phase1"));
    Row.Kind = EDrBossAttackKind::Swing;
    Row.Anim = FName(TEXT("Swing2_Medium"));
    Row.Weight = 25;
    Row.Cooldown = FTimespan::FromSeconds(2000);
    Row.Damage = 18;
    Row.StaminaDamage = 8;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(350);
    Row.Duration = FTimespan::FromSeconds(1130);
    Row.ActiveStart = FTimespan::FromSeconds(500);
    Row.ActiveEnd = FTimespan::FromSeconds(780);
    Row.PoiseDamage = 20;
    Row.HitRadius = 180.0f;
    Row.HitForwardOffset = 130.0f;
    Row.MinRange = 0.0f;
    Row.MaxRange = 320.0f;
    Row.MoveDistance = 0.0f;
    Row.Heal = 0;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossAttackRow Row;
    Row.Id = 103;
    Row.PatternSet = FName(TEXT("Boss_Phase1"));
    Row.Kind = EDrBossAttackKind::Swing;
    Row.Anim = FName(TEXT("Swing3_Slow"));
    Row.Weight = 15;
    Row.Cooldown = FTimespan::FromSeconds(4000);
    Row.Damage = 24;
    Row.StaminaDamage = 12;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(650);
    Row.Duration = FTimespan::FromSeconds(1700);
    Row.ActiveStart = FTimespan::FromSeconds(950);
    Row.ActiveEnd = FTimespan::FromSeconds(1250);
    Row.PoiseDamage = 30;
    Row.HitRadius = 210.0f;
    Row.HitForwardOffset = 140.0f;
    Row.MinRange = 0.0f;
    Row.MaxRange = 350.0f;
    Row.MoveDistance = 0.0f;
    Row.Heal = 0;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossAttackRow Row;
    Row.Id = 104;
    Row.PatternSet = FName(TEXT("Boss_Phase1"));
    Row.Kind = EDrBossAttackKind::Dash;
    Row.Anim = FName(TEXT("Speedburst"));
    Row.Weight = 15;
    Row.Cooldown = FTimespan::FromSeconds(5000);
    Row.Damage = 20;
    Row.StaminaDamage = 10;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(300);
    Row.Duration = FTimespan::FromSeconds(1270);
    Row.ActiveStart = FTimespan::FromSeconds(850);
    Row.ActiveEnd = FTimespan::FromSeconds(1100);
    Row.PoiseDamage = 25;
    Row.HitRadius = 200.0f;
    Row.HitForwardOffset = 130.0f;
    Row.MinRange = 350.0f;
    Row.MaxRange = 1200.0f;
    Row.MoveDistance = 750.0f;
    Row.Heal = 0;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossAttackRow Row;
    Row.Id = 105;
    Row.PatternSet = FName(TEXT("Boss_Phase1"));
    Row.Kind = EDrBossAttackKind::Slam;
    Row.Anim = FName(TEXT("Subjugation"));
    Row.Weight = 10;
    Row.Cooldown = FTimespan::FromSeconds(6000);
    Row.Damage = 28;
    Row.StaminaDamage = 15;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(1200);
    Row.Duration = FTimespan::FromSeconds(2870);
    Row.ActiveStart = FTimespan::FromSeconds(1650);
    Row.ActiveEnd = FTimespan::FromSeconds(2050);
    Row.PoiseDamage = 40;
    Row.HitRadius = 260.0f;
    Row.HitForwardOffset = 130.0f;
    Row.MinRange = 0.0f;
    Row.MaxRange = 380.0f;
    Row.MoveDistance = 0.0f;
    Row.Heal = 0;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossAttackRow Row;
    Row.Id = 201;
    Row.PatternSet = FName(TEXT("Boss_Phase2"));
    Row.Kind = EDrBossAttackKind::Swing;
    Row.Anim = FName(TEXT("Swing1_Medium"));
    Row.Weight = 30;
    Row.Cooldown = FTimespan::FromSeconds(2000);
    Row.Damage = 18;
    Row.StaminaDamage = 8;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(350);
    Row.Duration = FTimespan::FromSeconds(1130);
    Row.ActiveStart = FTimespan::FromSeconds(500);
    Row.ActiveEnd = FTimespan::FromSeconds(780);
    Row.PoiseDamage = 20;
    Row.HitRadius = 180.0f;
    Row.HitForwardOffset = 130.0f;
    Row.MinRange = 0.0f;
    Row.MaxRange = 320.0f;
    Row.MoveDistance = 0.0f;
    Row.Heal = 0;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossAttackRow Row;
    Row.Id = 202;
    Row.PatternSet = FName(TEXT("Boss_Phase2"));
    Row.Kind = EDrBossAttackKind::Swing;
    Row.Anim = FName(TEXT("Swing2_Medium"));
    Row.Weight = 25;
    Row.Cooldown = FTimespan::FromSeconds(2000);
    Row.Damage = 18;
    Row.StaminaDamage = 8;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(350);
    Row.Duration = FTimespan::FromSeconds(1130);
    Row.ActiveStart = FTimespan::FromSeconds(500);
    Row.ActiveEnd = FTimespan::FromSeconds(780);
    Row.PoiseDamage = 20;
    Row.HitRadius = 180.0f;
    Row.HitForwardOffset = 130.0f;
    Row.MinRange = 0.0f;
    Row.MaxRange = 320.0f;
    Row.MoveDistance = 0.0f;
    Row.Heal = 0;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossAttackRow Row;
    Row.Id = 203;
    Row.PatternSet = FName(TEXT("Boss_Phase2"));
    Row.Kind = EDrBossAttackKind::Swing;
    Row.Anim = FName(TEXT("Swing3_Slow"));
    Row.Weight = 15;
    Row.Cooldown = FTimespan::FromSeconds(4000);
    Row.Damage = 24;
    Row.StaminaDamage = 12;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(650);
    Row.Duration = FTimespan::FromSeconds(1700);
    Row.ActiveStart = FTimespan::FromSeconds(950);
    Row.ActiveEnd = FTimespan::FromSeconds(1250);
    Row.PoiseDamage = 30;
    Row.HitRadius = 210.0f;
    Row.HitForwardOffset = 140.0f;
    Row.MinRange = 0.0f;
    Row.MaxRange = 350.0f;
    Row.MoveDistance = 0.0f;
    Row.Heal = 0;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossAttackRow Row;
    Row.Id = 204;
    Row.PatternSet = FName(TEXT("Boss_Phase2"));
    Row.Kind = EDrBossAttackKind::Dash;
    Row.Anim = FName(TEXT("Speedburst"));
    Row.Weight = 15;
    Row.Cooldown = FTimespan::FromSeconds(5000);
    Row.Damage = 20;
    Row.StaminaDamage = 10;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(300);
    Row.Duration = FTimespan::FromSeconds(1270);
    Row.ActiveStart = FTimespan::FromSeconds(850);
    Row.ActiveEnd = FTimespan::FromSeconds(1100);
    Row.PoiseDamage = 25;
    Row.HitRadius = 200.0f;
    Row.HitForwardOffset = 130.0f;
    Row.MinRange = 350.0f;
    Row.MaxRange = 1200.0f;
    Row.MoveDistance = 750.0f;
    Row.Heal = 0;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossAttackRow Row;
    Row.Id = 205;
    Row.PatternSet = FName(TEXT("Boss_Phase2"));
    Row.Kind = EDrBossAttackKind::Slam;
    Row.Anim = FName(TEXT("Subjugation"));
    Row.Weight = 10;
    Row.Cooldown = FTimespan::FromSeconds(6000);
    Row.Damage = 28;
    Row.StaminaDamage = 15;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(1200);
    Row.Duration = FTimespan::FromSeconds(2870);
    Row.ActiveStart = FTimespan::FromSeconds(1650);
    Row.ActiveEnd = FTimespan::FromSeconds(2050);
    Row.PoiseDamage = 40;
    Row.HitRadius = 260.0f;
    Row.HitForwardOffset = 130.0f;
    Row.MinRange = 0.0f;
    Row.MaxRange = 380.0f;
    Row.MoveDistance = 0.0f;
    Row.Heal = 0;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossAttackRow Row;
    Row.Id = 206;
    Row.PatternSet = FName(TEXT("Boss_Phase2"));
    Row.Kind = EDrBossAttackKind::Swing;
    Row.Anim = FName(TEXT("Swing3_Medium"));
    Row.Weight = 20;
    Row.Cooldown = FTimespan::FromSeconds(3000);
    Row.Damage = 22;
    Row.StaminaDamage = 10;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(300);
    Row.Duration = FTimespan::FromSeconds(1130);
    Row.ActiveStart = FTimespan::FromSeconds(500);
    Row.ActiveEnd = FTimespan::FromSeconds(780);
    Row.PoiseDamage = 25;
    Row.HitRadius = 200.0f;
    Row.HitForwardOffset = 140.0f;
    Row.MinRange = 0.0f;
    Row.MaxRange = 340.0f;
    Row.MoveDistance = 0.0f;
    Row.Heal = 0;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossAttackRow Row;
    Row.Id = 207;
    Row.PatternSet = FName(TEXT("Boss_Phase2"));
    Row.Kind = EDrBossAttackKind::Slam;
    Row.Anim = FName(TEXT("Swing_In_Air"));
    Row.Weight = 15;
    Row.Cooldown = FTimespan::FromSeconds(5000);
    Row.Damage = 26;
    Row.StaminaDamage = 15;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(450);
    Row.Duration = FTimespan::FromSeconds(1600);
    Row.ActiveStart = FTimespan::FromSeconds(1000);
    Row.ActiveEnd = FTimespan::FromSeconds(1300);
    Row.PoiseDamage = 35;
    Row.HitRadius = 250.0f;
    Row.HitForwardOffset = 130.0f;
    Row.MinRange = 250.0f;
    Row.MaxRange = 700.0f;
    Row.MoveDistance = 400.0f;
    Row.Heal = 0;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossAttackRow Row;
    Row.Id = 208;
    Row.PatternSet = FName(TEXT("Boss_Phase2"));
    Row.Kind = EDrBossAttackKind::Siphon;
    Row.Anim = FName(TEXT("Soul_Siphon"));
    Row.Weight = 12;
    Row.Cooldown = FTimespan::FromSeconds(8000);
    Row.Damage = 15;
    Row.StaminaDamage = 10;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(650);
    Row.Duration = FTimespan::FromSeconds(1830);
    Row.ActiveStart = FTimespan::FromSeconds(1050);
    Row.ActiveEnd = FTimespan::FromSeconds(1450);
    Row.PoiseDamage = 15;
    Row.HitRadius = 450.0f;
    Row.HitForwardOffset = 0.0f;
    Row.MinRange = 0.0f;
    Row.MaxRange = 450.0f;
    Row.MoveDistance = 0.0f;
    Row.Heal = 100;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossAttackRow Row;
    Row.Id = 209;
    Row.PatternSet = FName(TEXT("Boss_Phase2"));
    Row.Kind = EDrBossAttackKind::Area;
    Row.Anim = FName(TEXT("Ultimate_Swing_120fps"));
    Row.Weight = 8;
    Row.Cooldown = FTimespan::FromSeconds(10000);
    Row.Damage = 32;
    Row.StaminaDamage = 20;
    Row.Effect = 0;
    Row.TelegraphTime = FTimespan::FromSeconds(1100);
    Row.Duration = FTimespan::FromSeconds(2630);
    Row.ActiveStart = FTimespan::FromSeconds(1550);
    Row.ActiveEnd = FTimespan::FromSeconds(2050);
    Row.PoiseDamage = 45;
    Row.HitRadius = 550.0f;
    Row.HitForwardOffset = 0.0f;
    Row.MinRange = 0.0f;
    Row.MaxRange = 550.0f;
    Row.MoveDistance = 0.0f;
    Row.Heal = 0;
    Asset->Rows.Add(Row);
   }
   Asset->PrimaryKeys = {101, 102, 103, 104, 105, 201, 202, 203, 204, 205, 206, 207, 208, 209};
   Asset->PatternSet_Keys = {FName(TEXT("Boss_Phase1")), FName(TEXT("Boss_Phase2"))};
   Asset->PatternSet_Offsets = {0, 5, 14};
   Asset->PatternSet_Indices = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
   Data->ExtraAssets.Add(TSoftObjectPtr<UPrimaryDataAsset>(Asset));
  }
  {
   auto* Asset = NewObject<UDrBossPhaseTable>(Data.Get());
   Asset->SchemaHash = TEXT("sha256:b17313189286346d99632f29af027b5dd711c87fa46536941da3d253f2c95f7f");
   {
    FDrBossPhaseRow Row;
    Row.Id = 1;
    Row.Boss = FName(TEXT("Boss"));
    Row.Phase = 1;
    Row.EnterHpRate = 100;
    Row.PatternSet = FName(TEXT("Boss_Phase1"));
    Row.MoveSpeedRate = 100;
    Asset->Rows.Add(Row);
   }
   {
    FDrBossPhaseRow Row;
    Row.Id = 2;
    Row.Boss = FName(TEXT("Boss"));
    Row.Phase = 2;
    Row.EnterHpRate = 50;
    Row.PatternSet = FName(TEXT("Boss_Phase2"));
    Row.MoveSpeedRate = 120;
    Asset->Rows.Add(Row);
   }
   Asset->PrimaryKeys = {1, 2};
   Data->ExtraAssets.Add(TSoftObjectPtr<UPrimaryDataAsset>(Asset));
  }
  {
   auto* Asset = NewObject<UDrEffectDefsTable>(Data.Get());
   Asset->SchemaHash = TEXT("sha256:27e7204999f76f7197e036a76f78727820eac0292361df41ad61358d210b3c2f");
   {
    FDrEffectDefsRow Row;
    Row.Id = 2001;
    Row.Category = EDrEffectCategory::Debuff;
    Row.ApplyTo = EDrEffectApplyTo::Victim;
    Row.Duration = 3.0f;
    Row.DamagePerTick = 5.0f;
    Row.Period = 1.0f;
    Row.Magnitude = 0.0f;
    Row.MoveSpeedMultiplier = 1.0f;
    Row.DamageTakenMultiplier = 1.0f;
    Row.DamageDealtMultiplier = 1.0f;
    Row.StackBonus_Start = 0;
    Row.StackBonus_Num = 3;
    Row.DisplayName = FText::FromString(TEXT("화상 효과"));
    Row.Icon = FSoftObjectPath(TEXT(""));
    Row.StateTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Debuff.Burn")), false);
    Row.TriggerTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Data.Damage")), false);
    Row.ProcInstantDamageCurHpPct = 10.0f;
    Row.ProcPoiseDamage = 0.0f;
    Row.StackRule = EDrEffectStackRule::Accumulate;
    Asset->Rows.Add(Row);
   }
   {
    FDrEffectDefsRow Row;
    Row.Id = 2002;
    Row.Category = EDrEffectCategory::Debuff;
    Row.ApplyTo = EDrEffectApplyTo::Victim;
    Row.Duration = 4.0f;
    Row.DamagePerTick = 0.0f;
    Row.Period = 0.0f;
    Row.Magnitude = 0.0f;
    Row.MoveSpeedMultiplier = 0.5f;
    Row.DamageTakenMultiplier = 1.0f;
    Row.DamageDealtMultiplier = 1.0f;
    Row.StackBonus_Start = 3;
    Row.StackBonus_Num = 3;
    Row.DisplayName = FText::FromString(TEXT("빙결 효과"));
    Row.Icon = FSoftObjectPath(TEXT(""));
    Row.StateTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Debuff.Chill")), false);
    Row.TriggerTag = FGameplayTag();
    Row.ProcInstantDamageCurHpPct = 0.0f;
    Row.ProcPoiseDamage = 0.0f;
    Row.StackRule = EDrEffectStackRule::Accumulate;
    Asset->Rows.Add(Row);
   }
   {
    FDrEffectDefsRow Row;
    Row.Id = 2003;
    Row.Category = EDrEffectCategory::Buff;
    Row.ApplyTo = EDrEffectApplyTo::Caster;
    Row.Duration = 8.0f;
    Row.DamagePerTick = 0.0f;
    Row.Period = 0.0f;
    Row.Magnitude = 120.0f;
    Row.MoveSpeedMultiplier = 1.0f;
    Row.DamageTakenMultiplier = 1.0f;
    Row.DamageDealtMultiplier = 1.25f;
    Row.StackBonus_Start = 6;
    Row.StackBonus_Num = 3;
    Row.DisplayName = FText::FromString(TEXT("축복 효과"));
    Row.Icon = FSoftObjectPath(TEXT(""));
    Row.StateTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Buff.Shield")), false);
    Row.TriggerTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Data.PoiseDamage")), false);
    Row.ProcInstantDamageCurHpPct = 0.0f;
    Row.ProcPoiseDamage = 20.0f;
    Row.StackRule = EDrEffectStackRule::Refresh;
    Asset->Rows.Add(Row);
   }
   Asset->PrimaryKeys = {2001, 2002, 2003};
   Asset->Category_Keys = {EDrEffectCategory::Buff, EDrEffectCategory::Debuff};
   Asset->Category_Offsets = {0, 1, 3};
   Asset->Category_Indices = {2, 0, 1};
   Asset->StackBonus_Pool = {0, 5, 10, 0, 3, 6, 0, 10, 20};
   Data->ExtraAssets.Add(TSoftObjectPtr<UPrimaryDataAsset>(Asset));
  }
  Data->LoadAllTables();
  UDrTableSubsystem::SetOverride(Data.Get());
 }
 ~FFixture() { UDrTableSubsystem::ClearOverride(); }
};
}

