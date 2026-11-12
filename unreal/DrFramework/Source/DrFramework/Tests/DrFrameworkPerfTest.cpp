// 성능 측정 틀의 구간 계산과 표본 요약을 검증한다.
#if WITH_DEV_AUTOMATION_TESTS
#include "Debug/DrPerfSubsystem.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrPerfSectionAtTest, "DrFramework.Framework.Perf.SectionAt",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDrPerfSectionAtTest::RunTest(const FString &) {
  int32 Section = -1;
  float Alpha = -1;
  TestTrue(TEXT("시작"), UDrPerfSubsystem::SectionAt(0, 15, 4, Section, Alpha));
  TestEqual(TEXT("시작 구간"), Section, 0);
  TestEqual(TEXT("시작 진행률"), Alpha, 0.f);
  TestTrue(TEXT("둘째 구간 중간"), UDrPerfSubsystem::SectionAt(22.5, 15, 4, Section, Alpha));
  TestEqual(TEXT("둘째 구간"), Section, 1);
  TestEqual(TEXT("중간 진행률"), Alpha, .5f);
  TestTrue(TEXT("마지막 구간 끝 직전"), UDrPerfSubsystem::SectionAt(59.999, 15, 4, Section, Alpha));
  TestEqual(TEXT("마지막 구간"), Section, 3);
  TestFalse(TEXT("모든 구간이 끝남"), UDrPerfSubsystem::SectionAt(60, 15, 4, Section, Alpha));
  TestFalse(TEXT("구간 없음"), UDrPerfSubsystem::SectionAt(1, 15, 0, Section, Alpha));
  TestFalse(TEXT("구간 길이 0"), UDrPerfSubsystem::SectionAt(1, 0, 4, Section, Alpha));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrPerfSummarizeTest, "DrFramework.Framework.Perf.Summarize",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDrPerfSummarizeTest::RunTest(const FString &) {
  TArray<double> Values;
  for (int32 I = 20; I >= 1; --I) Values.Add(I);   // 1..20 역순: 정렬 후 계산하는지 확인
  const auto Stats = UDrPerfSubsystem::Summarize(Values);
  TestEqual(TEXT("표본 수"), Stats->GetNumberField(TEXT("samples")), 20.0);
  TestEqual(TEXT("평균"), Stats->GetNumberField(TEXT("mean_ms")), 10.5);
  TestEqual(TEXT("p95 = ceil(20*.95)-1 번째 = 19"), Stats->GetNumberField(TEXT("p95_ms")), 19.0);
  TestEqual(TEXT("최댓값"), Stats->GetNumberField(TEXT("max_ms")), 20.0);
  const auto Empty = UDrPerfSubsystem::Summarize({});
  TestTrue(TEXT("빈 표본은 null"), Empty->TryGetField(TEXT("p95_ms")).IsValid() && Empty->TryGetField(TEXT("p95_ms"))->IsNull());
  return true;
}
#endif
