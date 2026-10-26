// 개발 빌드에서 프레임워크 동작을 점검하는 콘솔 명령을 등록한다.
#include "Backend/DrFakeServer.h"
#include "Content/DrContentSubsystem.h"
#include "Content/DrPlayerProfileContent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Network/DrNetworkSubsystem.h"
#include "DrTableSubsystem.h"
namespace {
void Ping(const TArray<FString> &, UWorld *World) {
  if (!World)
    return;
  if (UDrNetworkSubsystem *NetworkSubsystem =
          World->GetGameInstance()->GetSubsystem<UDrNetworkSubsystem>())
    NetworkSubsystem->SendRequest(EDrNetMessageType::Ping, FDrEmptyPayload(),
                                  FDrResponseDelegate());
}
void Profile(const TArray<FString> &, UWorld *World) {
  if (!World)
    return;
  if (UDrContentSubsystem *ContentSubsystem =
          World->GetGameInstance()->GetSubsystem<UDrContentSubsystem>())
    ContentSubsystem->GetPlayerProfile()->RequestProfile();
}
FAutoConsoleCommandWithWorldAndArgs
    PingCmd(TEXT("Dr.Net.Ping"), TEXT("가짜 서버에 Ping을 보냅니다."),
            FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Ping));
FAutoConsoleCommandWithWorldAndArgs
    ProfileCmd(TEXT("Dr.Profile.Request"), TEXT("프로필을 다시 요청합니다."),
               FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Profile));
FAutoConsoleCommand
    ReloadCmd(TEXT("Dr.Tables.Reload"), TEXT("정적 테이블을 다시 읽습니다."),
              FConsoleCommandDelegate::CreateLambda([]() {
                if (UDrTableSubsystem *Tables = UDrTableSubsystem::Get()) {
                  Tables->RegisterAutoTables();
                  Tables->ReloadAllTables();
                }
              }));
FAutoConsoleCommandWithWorldAndArgs LatencyCmd(
    TEXT("Dr.Net.SetLatency"), TEXT("지연 설정: base jitter failRate"),
    FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
        [](const TArray<FString> &Args, UWorld *World) {
          if (!World || Args.Num() != 3)
            return;
          if (UDrNetworkSubsystem *NetworkSubsystem =
                  World->GetGameInstance()->GetSubsystem<UDrNetworkSubsystem>())
            NetworkSubsystem->SetLatency(FCString::Atof(*Args[0]),
                                         FCString::Atof(*Args[1]),
                                         FCString::Atof(*Args[2]));
        }));
} // namespace
