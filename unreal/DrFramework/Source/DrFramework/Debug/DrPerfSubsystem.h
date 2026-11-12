#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "Dom/JsonObject.h"
#include "DrPerfSubsystem.generated.h"

/**
 * 패키지 성능 측정 틀. 실행 인자 -DrPerfCapture가 있으면 준비(PrepareCapture) → 예열 → 구간별 측정 → 결과 JSON
 * (Saved/Perf/result.json) → 종료 순으로 진행한다. 게임은 준비(맵 이동·카메라)와 구간마다 카메라 이동만 제공한다.
 * Tools/perf_win64.ps1이 이 인자로 패키지를 실행하고 결과를 검사한다.
 */
UCLASS()
class DRFRAMEWORK_API UDrPerfSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()
public:
  static bool IsCaptureRequested();
  /** 파생 클래스가 있으면 기반은 만들지 않는다. */
  virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
  virtual void Initialize(FSubsystemCollectionBase& Collection) override;
  virtual void Deinitialize() override;

  /** 표본을 정렬해 평균·p95(ceil(N*.95)-1 번째)·최댓값을 만든다. 빈 표본은 null. */
  static TSharedPtr<FJsonObject> Summarize(TArray<double> Values);
  /** 측정 시작 뒤 경과 시간이 속한 구간과 구간 진행률(0~1). 모든 구간이 끝났으면 false. */
  static bool SectionAt(double Elapsed, float SectionSeconds, int32 NumSections, int32& OutSection, float& OutAlpha);

protected:
  /** 측정 준비. 맵 이동·카메라 생성 등을 하고, 준비되면 true를 돌려준다. OutError를 채우면 측정을 실패로 끝낸다. */
  virtual bool PrepareCapture(UWorld& World, FString& OutError) { return true; }
  /** 예열(Section = INDEX_NONE)과 측정 중 매 틱 부른다. Alpha는 구간 진행률(0~1). 카메라를 옮긴다. */
  virtual void UpdateCapture(UWorld& World, int32 Section, float Alpha) {}
  /** 결과 JSON에 게임 정보를 더한다. */
  virtual void AddResultFields(FJsonObject& Root) const {}

  /** 구간 이름(측정 순서). 비어 있으면 측정은 실패로 끝난다. */
  TArray<FString> SectionNames;
  float WarmupSeconds = 10;
  float SectionSeconds = 15;
  float PrepareTimeoutSeconds = 180;
  /** 구간 p95 프레임 시간이 이 값을 넘으면 over_budget(측정 실패는 아님). */
  double FrameBudgetMs = 16.7;
  /** 예산을 넘은 구간에 candidate_causes로 남길 안내. */
  FString OverBudgetHint;

private:
  struct FSample { double Frame = 0; float Game = 0, GPU = 0; };
  bool Tick(float DeltaSeconds);
  bool Finish(const FString& Error = FString());

  FTSTicker::FDelegateHandle Ticker;
  TArray<TArray<FSample>> Samples;
  double Started = 0, WarmupStarted = 0, CaptureStarted = 0, LastFrame = 0;
  bool bPrepared = false;
};
