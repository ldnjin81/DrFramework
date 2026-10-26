// 로컬 요청 큐와 프로필 저장소를 사용하는 가짜 서버를 구현한다.
#include "Backend/DrFakeServer.h"
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
  if (const auto* Handler = Handlers.Find(Request.Type)) {
    const FDrNetResponse Response = (*Handler)(Request);
    OnPacketToClient.Broadcast(MakeResponse(Request, Response.Result, Response.Payload));
  } else if (Request.Type == FName(TEXT("Ping"))) {
    FDrPingPayload Payload; Payload.Message = TEXT("pong");
    OnPacketToClient.Broadcast(MakeResponse(Request, EDrNetResult::Ok, FInstancedStruct::Make(Payload)));
  } else {
    OnPacketToClient.Broadcast(MakeResponse(Request, EDrNetResult::InvalidRequest, FInstancedStruct()));
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
