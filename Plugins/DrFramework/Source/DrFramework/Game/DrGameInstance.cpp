// 연결, 로그인, 초기 프로필 요청으로 이어지는 부트 흐름을 구현한다.
#include "Game/DrGameInstance.h"
#include "Content/DrContentSubsystem.h"
#include "Content/DrPlayerProfileContent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Network/DrNetworkSubsystem.h"
#include "DrFramework.h"
#include "DrTableSubsystem.h"
#include "TimerManager.h"
void UDrGameInstance::Init() {
  Super::Init();
  // DrTable 플러그인이 등록된 테이블을 설정된 루트에서 로드한다(첫 조회 시 자동). 부팅 때 미리 로드해 둔다.
  UDrTableSubsystem::Get();
  bAutoSmoke = FParse::Param(FCommandLine::Get(), TEXT("DrAutoNetSmoke"));
}
void UDrGameInstance::OnStart() {
  Super::OnStart();
  bLoginRequestInFlight = false;
  bInitialDataRequestInFlight = false;
  bBootCompleted = false;
  if (bAutoSmoke && GetWorld())
    GetWorld()->GetTimerManager().SetTimer(SmokeTimeoutHandle, this,
                                           &UDrGameInstance::HandleSmokeTimeout,
                                           30.f, false);
  StartBootFlow();
}
void UDrGameInstance::Shutdown() {
  if (GetWorld())
    GetWorld()->GetTimerManager().ClearTimer(SmokeTimeoutHandle);
  if (UDrNetworkSubsystem *NetworkSubsystem =
          GetSubsystem<UDrNetworkSubsystem>()) {
    NetworkSubsystem->OnConnectionStateChanged.RemoveAll(this);
    NetworkSubsystem->Disconnect();
  }
  Super::Shutdown();
}
UDrGameInstance *UDrGameInstance::Get(const UObject *WorldContextObject) {
  return WorldContextObject
             ? Cast<UDrGameInstance>(
                   UGameplayStatics::GetGameInstance(WorldContextObject))
             : nullptr;
}
void UDrGameInstance::StartBootFlow() {
  UDrNetworkSubsystem *NetworkSubsystem = GetSubsystem<UDrNetworkSubsystem>();
  if (!NetworkSubsystem)
    return;
  NetworkSubsystem->OnConnectionStateChanged.RemoveAll(this);
  NetworkSubsystem->OnConnectionStateChanged.AddUObject(
      this, &UDrGameInstance::HandleConnectionStateChanged);
  NetworkSubsystem->Connect();
}
void UDrGameInstance::HandleConnectionStateChanged(
    EDrConnectionState NewState) {
  if (NewState != EDrConnectionState::Connected)
    return;
  if (bAutoSmoke) {
    UDrNetworkSubsystem *NetworkSubsystem = GetSubsystem<UDrNetworkSubsystem>();
    NetworkSubsystem->SendRequest(
        EDrNetMessageType::Ping, FDrEmptyPayload(),
        FDrResponseDelegate::CreateLambda(
            [this](const FDrNetResponse &Response) {
              ReportSmoke(TEXT("Ping"), Response.Result == EDrNetResult::Ok);
              if (Response.Result == EDrNetResult::Ok)
                SendLoginRequest();
              else
                FGenericPlatformMisc::RequestExit(false);
            }));
  } else
    SendLoginRequest();
}
void UDrGameInstance::SendLoginRequest() {
  if (bLoginRequestInFlight || bBootCompleted)
    return;
  FDrLoginRequest Request;
  Request.UserId = TEXT("LocalPlayer");
  bLoginRequestInFlight = true;
  GetSubsystem<UDrNetworkSubsystem>()->SendRequest(
      EDrNetMessageType::Login, Request,
      FDrResponseDelegate::CreateUObject(
          this, &UDrGameInstance::HandleLoginResponse));
}
void UDrGameInstance::HandleLoginResponse(const FDrNetResponse &Response) {
  bLoginRequestInFlight = false;
  const bool bSucceeded =
      Response.Result == EDrNetResult::Ok &&
      !GetSubsystem<UDrNetworkSubsystem>()->GetSessionToken().IsEmpty();
  if (bAutoSmoke)
    ReportSmoke(TEXT("Login"), bSucceeded);
  if (bSucceeded)
    RequestInitialData();
  else if (bAutoSmoke)
    FGenericPlatformMisc::RequestExit(false);
}
void UDrGameInstance::RequestInitialData() {
  if (bInitialDataRequestInFlight || bBootCompleted)
    return;
  bInitialDataRequestInFlight = true;
  GetSubsystem<UDrNetworkSubsystem>()->SendRequest(
      EDrNetMessageType::RequestPlayerProfile, FDrEmptyPayload(),
      FDrResponseDelegate::CreateUObject(
          this, &UDrGameInstance::HandleInitialProfileResponse));
}
void UDrGameInstance::HandleInitialProfileResponse(
    const FDrNetResponse &Response) {
  bInitialDataRequestInFlight = false;
  const FDrPlayerProfileDataSet *ProfileDataSet =
      Response.Payload.GetPtr<FDrPlayerProfileDataSet>();
  const bool bSucceeded = Response.Result == EDrNetResult::Ok && ProfileDataSet;
  if (bSucceeded)
    if (UDrContentSubsystem *ContentSubsystem =
            GetSubsystem<UDrContentSubsystem>())
      ContentSubsystem->GetPlayerProfile()->ApplyProfileDataSet(
          *ProfileDataSet);
  bBootCompleted = bSucceeded;
  if (bAutoSmoke) {
    ReportSmoke(TEXT("Profile"), bSucceeded);
    FGenericPlatformMisc::RequestExit(false);
  } else if (bSucceeded)
    UE_LOG(LogDr, Log, TEXT("[GameInstance] 부트 플로우 완료."));
}
void UDrGameInstance::HandleSmokeTimeout() {
  UE_LOG(LogDr, Error, TEXT("[DrAutoNetSmoke] FAIL Timeout"));
  FGenericPlatformMisc::RequestExit(false);
}
void UDrGameInstance::ReportSmoke(const TCHAR *Step, bool bSucceeded) {
  if (bSucceeded) {
    UE_LOG(LogDr, Log, TEXT("[DrAutoNetSmoke] PASS %s"), Step);
  } else {
    UE_LOG(LogDr, Error, TEXT("[DrAutoNetSmoke] FAIL %s"), Step);
  }
}
