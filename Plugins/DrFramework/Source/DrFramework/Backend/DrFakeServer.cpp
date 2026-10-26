// 로컬 요청 큐와 프로필 저장소를 사용하는 가짜 서버를 구현한다.
#include "Backend/DrFakeServer.h"
#include "Content/DrPlayerProfileContent.h"
#include "DrFramework.h"

void UDrFakeServer::SubmitFromClient(const FDrNetPacket &Packet) {
  if (!bOnline) {
    OnPacketToClient.Broadcast(
        MakeResponse(Packet, EDrNetResult::NotConnected, FInstancedStruct()));
    return;
  }
  FDrQueuedPacket QueuedPacket;
  QueuedPacket.Packet = Packet;
  const double CurrentTime = FPlatformTime::Seconds();
  QueuedPacket.ArrivalTime = FMath::Max(
      CurrentTime + LatencyProfile.RollLatency(), LastReceiveArrivalTime);
  LastReceiveArrivalTime = QueuedPacket.ArrivalTime;
  QueuedPacket.bForceError = LatencyProfile.RollFailure();
  ReceiveQueue.Add(QueuedPacket);
}
void UDrFakeServer::OpenConnection() {
  if (bOnline) {
    OnEndpointConnectionChanged.Broadcast(EDrConnectionState::Connected);
    return;
  }
  bOnline = true;
  Store.Empty();
  ReceiveQueue.Empty();
  LastReceiveArrivalTime = 0.;
  ActiveSessionToken.Empty();
  // 원본 대응이 없는 축소 구현으로, 연결마다 프로필 저장소만 초기화한다.
  FDrPlayerProfileDataSet Profile;
  Profile.DisplayName = TEXT("Player");
  Store.Add(TEXT("PlayerProfile"), FInstancedStruct::Make(Profile));
  TickHandle = FTSTicker::GetCoreTicker().AddTicker(
      FTickerDelegate::CreateUObject(this, &UDrFakeServer::Tick));
  OnEndpointConnectionChanged.Broadcast(EDrConnectionState::Connected);
}
void UDrFakeServer::CloseConnection() {
  if (TickHandle.IsValid()) {
    FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
    TickHandle.Reset();
  }
  bOnline = false;
  ReceiveQueue.Empty();
  Store.Empty();
  ActiveSessionToken.Empty();
  LastReceiveArrivalTime = 0.;
  OnEndpointConnectionChanged.Broadcast(EDrConnectionState::Disconnected);
}
bool UDrFakeServer::Tick(float) {
  const double CurrentTime = FPlatformTime::Seconds();
  while (ReceiveQueue.Num() > 0 && ReceiveQueue[0].ArrivalTime <= CurrentTime) {
    const FDrQueuedPacket QueuedPacket = ReceiveQueue[0];
    ReceiveQueue.RemoveAt(0);
    ProcessRequest(QueuedPacket);
  }
  return true;
}
void UDrFakeServer::ProcessRequest(const FDrQueuedPacket &QueuedPacket) {
  const FDrNetPacket &Request = QueuedPacket.Packet;
  if (QueuedPacket.bForceError) {
    OnPacketToClient.Broadcast(
        MakeResponse(Request, EDrNetResult::Error, FInstancedStruct()));
    return;
  }
  switch (Request.Type) {
  case EDrNetMessageType::Ping: {
    FDrPingPayload PingPayload;
    PingPayload.Message = TEXT("pong");
    OnPacketToClient.Broadcast(MakeResponse(
        Request, EDrNetResult::Ok, FInstancedStruct::Make(PingPayload)));
    break;
  }
  case EDrNetMessageType::Login: {
    FDrLoginResponse LoginResponse;
    LoginResponse.SessionToken = TEXT("fake-session-token");
    ActiveSessionToken = LoginResponse.SessionToken;
    OnPacketToClient.Broadcast(MakeResponse(
        Request, EDrNetResult::Ok, FInstancedStruct::Make(LoginResponse)));
    break;
  }
  case EDrNetMessageType::RequestPlayerProfile: {
    const FInstancedStruct *ProfilePayload = Store.Find(TEXT("PlayerProfile"));
    OnPacketToClient.Broadcast(MakeResponse(
        Request,
        ProfilePayload ? EDrNetResult::Ok : EDrNetResult::InvalidRequest,
        ProfilePayload ? *ProfilePayload : FInstancedStruct()));
    break;
  }
  default:
    OnPacketToClient.Broadcast(MakeResponse(
        Request, EDrNetResult::InvalidRequest, FInstancedStruct()));
    break;
  }
}
FDrNetPacket
UDrFakeServer::MakeResponse(const FDrNetPacket &Request, EDrNetResult Result,
                            const FInstancedStruct &Payload) const {
  FDrNetPacket Response;
  Response.Type = Request.Type;
  Response.RequestId = Request.RequestId;
  Response.Payload = Payload;
  Response.Result = Result;
  Response.SendTimeSeconds = FPlatformTime::Seconds();
  return Response;
}
