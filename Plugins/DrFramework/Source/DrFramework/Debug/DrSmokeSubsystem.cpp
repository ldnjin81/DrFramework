#include "Debug/DrSmokeSubsystem.h"
#include "DrTableSubsystem.h"
#include "TableData/Generated/DrGameAssetsRow.h"

#include "Game/DrPlayerController.h"
#include "UI/DrMenuWidget.h"
#include "UI/DrTitleMenuWidget.h"
#include "UI/DrPauseMenuWidget.h"
#include "UI/DrSettingsMenuWidget.h"
#include "UI/DrCombatHUDWidget.h"
#include "Characters/DrCharacter.h"
#include "Boss/DrBossCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "UnrealClient.h"

void UDrSmokeSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
  Super::Initialize(Collection);
  if (!FParse::Param(FCommandLine::Get(), TEXT("DrSmokeTest"))) return;
  Started = StageStarted = FPlatformTime::Seconds();
  Ticker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::Tick));
}
void UDrSmokeSubsystem::Deinitialize() {
  FTSTicker::GetCoreTicker().RemoveTicker(Ticker);
  Super::Deinitialize();
}
void UDrSmokeSubsystem::Capture(const TCHAR* Name) {
  if (!FApp::CanEverRender() || !FParse::Param(FCommandLine::Get(), TEXT("DrMenuCapture"))) return;
  const FString Directory = FPaths::ProjectSavedDir() / TEXT("MenuVisual");
  IFileManager::Get().MakeDirectory(*Directory, true);
  FScreenshotRequest::RequestScreenshot(FPaths::ConvertRelativePathToFull(Directory / Name), true, false);
}
bool UDrSmokeSubsystem::Tick(float DeltaSeconds) {
  const double Now = FPlatformTime::Seconds();
  if (Now - Started > 120) { Steps.Add(TEXT("timeout"), false); Finish(false); return false; }
  UWorld* World = GetGameInstance()->GetWorld();
  auto* PC = World ? Cast<ADrPlayerController>(World->GetFirstPlayerController()) : nullptr;
  if (!PC) return true;
  switch (Stage) {
  case 0:
    if (!PC->IsTitleScreen()) { Steps.Add(TEXT("title"), false); Finish(false); return false; }
    Steps.Add(TEXT("title"), PC->IsMenuInput());
    Stage = 1; StageStarted = Now; break;
  case 1:
    if (Now - StageStarted < 2) break;
    if (FParse::Param(FCommandLine::Get(), TEXT("DrMenuCapture"))) {
      // 캡처 실행에서만 배경의 에디터 비동기 리소스 준비를 추가로 기다린다.
      if (Now - StageStarted < 6) break;
      Capture(TEXT("16_title.png")); Stage = 8; StageStarted = Now;
    } else { PC->StartGame(); Stage = 2; StageStarted = Now; }
    break;
  case 8:
    if (Now - StageStarted < .7) break;
    PC->StartGame(); Stage = 2; StageStarted = Now; break;
  case 2:
    if (PC->IsTitleScreen() || !World->HasBegunPlay()) break;
    Steps.Add(TEXT("arena"), UGameplayStatics::GetCurrentLevelName(World).Contains(TEXT("L_BossArena")));
    {
      // 기본 C++ 캐릭터(메시 없음)로 대체 스폰돼도 통과하던 문제(2026-10-24): 블루프린트 캐릭터와 메시까지 확인한다.
      const ADrCharacter* Player = Cast<ADrCharacter>(PC->GetPawn());
      Steps.Add(TEXT("player"), IsValid(Player) && !Player->GetClass()->HasAnyClassFlags(CLASS_Native) &&
                                    Player->GetMesh() && Player->GetMesh()->GetSkeletalMeshAsset());
      UDrTableSubsystem* Tables = UDrTableSubsystem::Get();
      Steps.Add(TEXT("tables"), Tables && Tables->AreAllTablesLoaded());
      const auto Rows = FDrGameAssetsRow::GetAll();
      bool bAssets = Rows.Num() >= 4;
      for (const FDrGameAssetsRow& Row : Rows) {
        const bool bFound = !Row.Path.IsNull() && Row.Path.TryLoad() != nullptr;
        if (!bFound) UE_LOG(LogTemp, Error, TEXT("[Smoke] 에셋 없음: %s = %s"), *Row.Name.ToString(), *Row.Path.ToString());
        bAssets &= bFound;
      }
      Steps.Add(TEXT("assets"), bAssets);
    }
    Steps.Add(TEXT("hud"), IsValid(PC->GetCombatHUD()));
    Steps.Add(TEXT("boss"), TActorIterator<ADrBossCharacter>(World) ? true : false);
    Stage = 3; StageStarted = LastFrame = Now; break;
  case 3:
    // 티커의 DeltaSeconds에 이전 맵 로드가 포함될 수 있어 현재 월드 표본의 실시간 간격을 쓴다.
    FrameSeconds += Now - LastFrame; LastFrame = Now; ++Frames;
    if (Now - StageStarted < 2) break;
    Steps.Add(TEXT("pause_open"), PC->OpenPauseMenu() && UGameplayStatics::IsGamePaused(World) && PC->IsMenuInput());
    Stage = 4; StageStarted = Now; break;
  case 4:
    if (Now - StageStarted < .7) break;
    Capture(TEXT("17_pause.png")); Stage = 5; StageStarted = Now; break;
  case 5:
    if (Now - StageStarted < .7) break;
    PC->OpenSettings(); Steps.Add(TEXT("settings"), IsValid(PC->GetSettingsMenu()));
    Stage = 6; StageStarted = Now; break;
  case 6:
    if (Now - StageStarted < .7) break;
    Capture(TEXT("18_settings.png")); Stage = 7; StageStarted = Now; break;
  case 7:
    if (Now - StageStarted < .7) break;
    PC->BackMenu(); PC->ClosePauseMenu();
    Steps.Add(TEXT("pause_close"), !UGameplayStatics::IsGamePaused(World) && !PC->IsMenuInput());
    { bool bPassed = true; for (const auto& Step : Steps) bPassed &= Step.Value; Finish(bPassed); }
    return false;
  }
  return true;
}
void UDrSmokeSubsystem::Finish(bool bPassed) {
  auto Root = MakeShared<FJsonObject>();
  auto Results = MakeShared<FJsonObject>();
  for (const auto& Step : Steps) Results->SetBoolField(Step.Key, Step.Value);
  Root->SetBoolField(TEXT("passed"), bPassed);
  Root->SetObjectField(TEXT("steps"), Results);
  Root->SetNumberField(TEXT("average_frame_ms"), Frames ? FrameSeconds * 1000 / Frames : 0);
  Root->SetNumberField(TEXT("sample_frames"), Frames);
  FString Json;
  FJsonSerializer::Serialize(Root, TJsonWriterFactory<>::Create(&Json));
  const FString Directory = FPaths::ProjectSavedDir() / TEXT("Smoke");
  IFileManager::Get().MakeDirectory(*Directory, true);
  const bool bSaved = FFileHelper::SaveStringToFile(Json, *(Directory / TEXT("result.json")));
  FPlatformMisc::RequestExitWithStatus(false, bPassed && bSaved ? 0 : 1);
}
