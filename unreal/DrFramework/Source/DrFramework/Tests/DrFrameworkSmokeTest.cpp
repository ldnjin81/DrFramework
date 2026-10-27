// 스모크 결과 JSON이 등록한 단계 이름의 대소문자와 순서를 그대로 쓰는지 검증한다.
#if WITH_DEV_AUTOMATION_TESTS
#include "Debug/DrSmokeSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrSmokeResultKeysTest, "DrFramework.Framework.Smoke.ResultKeysKeepCaseAndOrder",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDrSmokeResultKeysTest::RunTest(const FString &) {
  // FString 비교는 기본이 대소문자 무시이므로 CaseSensitive로 검사한다.
  // 엔진에 대문자 표기의 FName이 먼저 있어도 결과 키는 등록한 표기를 유지해야 한다.
  const FName Existing(TEXT("Title"));
  TestEqual(TEXT("대문자 FName이 먼저 등록됨"), Existing.ToString(), FString(TEXT("Title")));
  TArray<TPair<FString, bool>> Steps;
  Steps.Emplace(TEXT("title"), true);
  Steps.Emplace(TEXT("pause_open"), true);
  Steps.Emplace(TEXT("settings"), false);
  FString Json;
  FJsonSerializer::Serialize(UDrSmokeSubsystem::MakeResultJson(false, Steps), TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Json));
  TestTrue(TEXT("소문자 키 유지"), Json.Contains(TEXT("\"title\""), ESearchCase::CaseSensitive));
  TestFalse(TEXT("대문자 키로 바뀌지 않음"), Json.Contains(TEXT("\"Title\""), ESearchCase::CaseSensitive));
  TestTrue(TEXT("등록 순서 유지"), Json.Find(TEXT("\"title\"")) < Json.Find(TEXT("\"pause_open\"")) &&
                                       Json.Find(TEXT("\"pause_open\"")) < Json.Find(TEXT("\"settings\"")));
  TestTrue(TEXT("전체 결과"), Json.Contains(TEXT("\"passed\":false")));
  return true;
}
#endif
