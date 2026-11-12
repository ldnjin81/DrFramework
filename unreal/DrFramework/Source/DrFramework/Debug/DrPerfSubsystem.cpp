#include "Debug/DrPerfSubsystem.h"
#include "Dom/JsonObject.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Scalability.h"
#include "Serialization/JsonSerializer.h"
#include "Debug/DrDebugSubsystems.h"
#include "UnrealClient.h"

bool UDrPerfSubsystem::IsCaptureRequested() { return FParse::Param(FCommandLine::Get(), TEXT("DrPerfCapture")); }

bool UDrPerfSubsystem::ShouldCreateSubsystem(UObject* Outer) const {
  return Super::ShouldCreateSubsystem(Outer) && (GetClass() != StaticClass() || !DrDebugSubsystems::HasCreatableDerived(StaticClass(), Outer));
}

void UDrPerfSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
  Super::Initialize(Collection);
  if (!IsCaptureRequested()) return;
  Started = FPlatformTime::Seconds();
  Ticker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::Tick));
}

void UDrPerfSubsystem::Deinitialize() {
  FTSTicker::GetCoreTicker().RemoveTicker(Ticker);
  Super::Deinitialize();
}

TSharedPtr<FJsonObject> UDrPerfSubsystem::Summarize(TArray<double> Values) {
  auto Result = MakeShared<FJsonObject>();
  Result->SetNumberField(TEXT("samples"), Values.Num());
  if (Values.IsEmpty()) {
    Result->SetField(TEXT("mean_ms"), MakeShared<FJsonValueNull>());
    Result->SetField(TEXT("p95_ms"), MakeShared<FJsonValueNull>());
    Result->SetField(TEXT("max_ms"), MakeShared<FJsonValueNull>());
    return Result;
  }
  Values.Sort();
  double Total = 0;
  for (double Value : Values) Total += Value;
  Result->SetNumberField(TEXT("mean_ms"), Total / Values.Num());
  Result->SetNumberField(TEXT("p95_ms"), Values[FMath::CeilToInt(Values.Num() * .95) - 1]);
  Result->SetNumberField(TEXT("max_ms"), Values.Last());
  return Result;
}

bool UDrPerfSubsystem::SectionAt(double Elapsed, float InSectionSeconds, int32 NumSections, int32& OutSection, float& OutAlpha) {
  if (NumSections <= 0 || InSectionSeconds <= 0 || Elapsed < 0 || Elapsed >= double(InSectionSeconds) * NumSections) return false;
  OutSection = FMath::Clamp(FMath::FloorToInt32(Elapsed / InSectionSeconds), 0, NumSections - 1);
  OutAlpha = FMath::Clamp(float((Elapsed - OutSection * double(InSectionSeconds)) / InSectionSeconds), 0.f, 1.f);
  return true;
}

bool UDrPerfSubsystem::Tick(float) {
  const double Now = FPlatformTime::Seconds();
  auto* World = GetGameInstance()->GetWorld();
  if (!bPrepared) {
    if (Now - Started > PrepareTimeoutSeconds) return Finish(TEXT("성능 측정 준비 시간 초과"));
    if (!World || !World->HasBegunPlay()) return true;
    FString Error;
    const bool bReady = PrepareCapture(*World, Error);
    if (!Error.IsEmpty()) return Finish(Error);
    if (!bReady) return true;
    if (SectionNames.IsEmpty()) return Finish(TEXT("측정 구간(SectionNames)이 없음"));
    Samples.SetNum(SectionNames.Num());
    // 이전 실행의 사용자 품질 저장값에 영향을 받지 않도록 엔진 기본값을 메모리에만 적용한다.
    Scalability::FQualityLevels DefaultQuality;
    DefaultQuality.SetDefaults();
    Scalability::SetQualityLevels(DefaultQuality);
    // FStatUnitData의 Raw 값은 stat unit이 그려질 때 갱신된다.
    if (auto* PC = World->GetFirstPlayerController()) {
      PC->ConsoleCommand(TEXT("stat unit"));
      PC->ConsoleCommand(TEXT("t.MaxFPS 0"));
      PC->ConsoleCommand(TEXT("r.VSync 0"));
    }
    bPrepared = true;
    WarmupStarted = Now;
  }
  if (!World) return Finish(TEXT("측정 중 월드가 사라짐"));
  if (Now - WarmupStarted < WarmupSeconds) {
    if (!UpdateCapture(*World, INDEX_NONE, 0)) return Finish(TEXT("측정 갱신 실패(UpdateCapture, 예열)"));
    return true;
  }
  if (!CaptureStarted) CaptureStarted = LastFrame = Now;
  int32 Section = 0;
  float Alpha = 0;
  if (!SectionAt(Now - CaptureStarted, SectionSeconds, SectionNames.Num(), Section, Alpha)) return Finish();
  // 카메라를 잃는 등 같은 조건으로 잴 수 없으면 실패로 끝낸다(다른 화면을 잰 수치를 완료로 남기지 않게).
  if (!UpdateCapture(*World, Section, Alpha)) return Finish(FString::Printf(TEXT("측정 갱신 실패(UpdateCapture, 구간 %d)"), Section));
  if (Now > LastFrame) {
    FSample Sample;
    Sample.Frame = (Now - LastFrame) * 1000;
    if (auto* Viewport = World->GetGameViewport()) if (const auto* Unit = Viewport->GetStatUnitData()) {
      Sample.Game = Unit->RawGameThreadTime;
      Sample.GPU = Unit->RawGPUFrameTime[0];
    }
    Samples[Section].Add(Sample);
  }
  LastFrame = Now;
  return true;
}

bool UDrPerfSubsystem::Finish(const FString& Error) {
  auto Root = MakeShared<FJsonObject>();
  Root->SetStringField(TEXT("error"), Error);
  Root->SetNumberField(TEXT("duration_seconds"), CaptureStarted ? FPlatformTime::Seconds() - CaptureStarted : 0);
  Root->SetStringField(TEXT("thread_source"), TEXT("FStatUnitData.RawGameThreadTime / RawGPUFrameTime[0]"));
  Root->SetStringField(TEXT("quality"), TEXT("엔진 기본 품질, VSync 및 프레임 제한 해제"));
  Root->SetNumberField(TEXT("frame_budget_ms"), FrameBudgetMs);
  const auto Quality = Scalability::GetQualityLevels();
  auto QualityJson = MakeShared<FJsonObject>();
  QualityJson->SetNumberField(TEXT("resolution_quality"), Quality.ResolutionQuality);
  QualityJson->SetNumberField(TEXT("view_distance"), Quality.ViewDistanceQuality);
  QualityJson->SetNumberField(TEXT("shadows"), Quality.ShadowQuality);
  QualityJson->SetNumberField(TEXT("global_illumination"), Quality.GlobalIlluminationQuality);
  QualityJson->SetNumberField(TEXT("reflections"), Quality.ReflectionQuality);
  QualityJson->SetNumberField(TEXT("foliage"), Quality.FoliageQuality);
  Root->SetObjectField(TEXT("scalability"), QualityJson);
  if (auto* World = GetGameInstance()->GetWorld()) if (auto* Viewport = World->GetGameViewport()) {
    const FIntPoint Size = Viewport->Viewport ? Viewport->Viewport->GetSizeXY() : FIntPoint::ZeroValue;
    Root->SetNumberField(TEXT("width"), Size.X);
    Root->SetNumberField(TEXT("height"), Size.Y);
  }
  TArray<TSharedPtr<FJsonValue>> Sections;
  bool bComplete = Error.IsEmpty() && !SectionNames.IsEmpty();
  for (int32 I = 0; I < SectionNames.Num(); ++I) {
    auto Section = MakeShared<FJsonObject>();
    Section->SetStringField(TEXT("name"), SectionNames[I]);
    TArray<double> Frame, Game, GPU;
    if (Samples.IsValidIndex(I)) for (const auto& Sample : Samples[I]) {
      Frame.Add(Sample.Frame);
      if (Sample.Game > 0) Game.Add(Sample.Game);
      if (Sample.GPU > 0) GPU.Add(Sample.GPU);
    }
    bComplete &= !Frame.IsEmpty() && !Game.IsEmpty() && !GPU.IsEmpty();
    auto Stats = Summarize(Frame);
    Section->SetObjectField(TEXT("frame"), Stats);
    Section->SetObjectField(TEXT("game_thread"), Summarize(Game));
    Section->SetObjectField(TEXT("gpu"), Summarize(GPU));
    const bool bOverBudget = !Frame.IsEmpty() && Stats->GetNumberField(TEXT("p95_ms")) > FrameBudgetMs;
    Section->SetBoolField(TEXT("over_budget"), bOverBudget);
    if (bOverBudget && !OverBudgetHint.IsEmpty()) Section->SetStringField(TEXT("candidate_causes"), OverBudgetHint);
    Sections.Add(MakeShared<FJsonValueObject>(Section));
  }
  Root->SetBoolField(TEXT("completed"), bComplete);
  Root->SetArrayField(TEXT("sections"), Sections);
  AddResultFields(*Root);
  FString Json;
  FJsonSerializer::Serialize(Root, TJsonWriterFactory<>::Create(&Json));
  const FString Directory = FPaths::ProjectSavedDir() / TEXT("Perf");
  IFileManager::Get().MakeDirectory(*Directory, true);
  const bool bSaved = FFileHelper::SaveStringToFile(Json, *(Directory / TEXT("result.json")));
  // 예산 초과는 측정 실패가 아니다. 표본 누락·저장 실패만 실패로 처리한다.
  FPlatformMisc::RequestExitWithStatus(false, bComplete && bSaved ? 0 : 1);
  Ticker.Reset();
  return false;
}
