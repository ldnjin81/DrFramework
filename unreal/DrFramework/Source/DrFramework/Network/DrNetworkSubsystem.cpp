// 요청 수명, 수신 지연 큐, 세션 상태를 관리하는 네트워크 서브시스템이다.
#include "Network/DrNetworkSubsystem.h"
#include "Backend/DrFakeServer.h"
#include "Network/DrServerEndpoint.h"

void UDrNetworkSubsystem::Initialize(FSubsystemCollectionBase &Collection) {
  Super::Initialize(Collection);
  if (!ServerEndpointClass)
    ServerEndpointClass = UDrFakeServer::StaticClass();
  ServerEndpoint = NewObject<UDrServerEndpoint>(this, ServerEndpointClass);
  ServerEndpoint->OnPacketToClient.AddUObject(
      this, &UDrNetworkSubsystem::HandlePacketToClient);
  ServerEndpoint->OnEndpointConnectionChanged.AddUObject(
      this, &UDrNetworkSubsystem::HandleEndpointConnectionChanged);
  TickHandle = FTSTicker::GetCoreTicker().AddTicker(
      FTickerDelegate::CreateUObject(this, &UDrNetworkSubsystem::Tick));
}
void UDrNetworkSubsystem::Deinitialize() {
  Disconnect();
  if (TickHandle.IsValid())
    FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
  PendingRequests.Empty();
  ReceiveQueue.Empty();
  ServerEndpoint = nullptr;
  Super::Deinitialize();
}
void UDrNetworkSubsystem::Connect() {
  if (ConnectionState == EDrConnectionState::Connected ||
      ConnectionState == EDrConnectionState::Connecting)
    return;
  if (!ServerEndpoint)
    return;
  SetConnectionState(EDrConnectionState::Connecting);
  ServerEndpoint->OpenConnection();
}
void UDrNetworkSubsystem::Disconnect() {
  if (ServerEndpoint)
    ServerEndpoint->CloseConnection();
  for (auto &Pair : PendingRequests) {
    FDrNetResponse Response;
    Response.Result = EDrNetResult::NotConnected;
    Pair.Value.Callback.ExecuteIfBound(Response);
  }
  PendingRequests.Empty();
  SessionToken.Empty();
  SetConnectionState(EDrConnectionState::Disconnected);
}
void UDrNetworkSubsystem::SetLatency(float BaseSeconds, float JitterSeconds,
                                     float FailureRate) {
  // 원본 대응이 없는 설정 경로로, 클라이언트와 가짜 서버에 같은 값을 반영한다.
  ClientLatency.BaseLatencySeconds = FMath::Max(0.f, BaseSeconds);
  ClientLatency.JitterSeconds = FMath::Max(0.f, JitterSeconds);
  ClientLatency.FailureRate = FMath::Clamp(FailureRate, 0.f, 1.f);
  if (UDrFakeServer *FakeServer = Cast<UDrFakeServer>(ServerEndpoint)) {
    FakeServer->LatencyProfile = ClientLatency;
  }
}
int64 UDrNetworkSubsystem::SendRequest(FName Type,
                                       const FInstancedStruct &Payload,
                                       FDrResponseDelegate OnResponse,
                                       float TimeoutSeconds) {
  if (ConnectionState != EDrConnectionState::Connected || !ServerEndpoint) {
    FDrNetResponse Response;
    Response.Result = EDrNetResult::NotConnected;
    OnResponse.ExecuteIfBound(Response);
    return 0;
  }
  const int64 RequestId = NextRequestId++;
  FDrPendingRequest PendingRequest;
  PendingRequest.Callback = OnResponse;
  PendingRequest.ExpireTimeSeconds =
      FPlatformTime::Seconds() + FMath::Max(.01f, TimeoutSeconds);
  PendingRequests.Add(RequestId, PendingRequest);
  FDrNetPacket Packet;
  Packet.Type = Type;
  Packet.RequestId = RequestId;
  Packet.Payload = Payload;
  Packet.SendTimeSeconds = FPlatformTime::Seconds();
  ServerEndpoint->SubmitFromClient(Packet);
  return RequestId;
}
void UDrNetworkSubsystem::HandlePacketToClient(const FDrNetPacket &Packet) {
  FDrQueuedClientPacket QueuedPacket;
  QueuedPacket.Packet = Packet;
  const double CurrentTime = FPlatformTime::Seconds();
  QueuedPacket.ArrivalTime = FMath::Max(
      CurrentTime + ClientLatency.RollLatency(), LastReceiveArrivalTime);
  LastReceiveArrivalTime = QueuedPacket.ArrivalTime;
  ReceiveQueue.Add(QueuedPacket);
}
void UDrNetworkSubsystem::HandleEndpointConnectionChanged(
    EDrConnectionState NewState) {
  SetConnectionState(NewState);
}
bool UDrNetworkSubsystem::Tick(float) {
  const double CurrentTime = FPlatformTime::Seconds();
  while (ReceiveQueue.Num() && ReceiveQueue[0].ArrivalTime <= CurrentTime) {
    const FDrNetPacket Packet = ReceiveQueue[0].Packet;
    ReceiveQueue.RemoveAt(0);
    Dispatch(Packet);
  }
  TArray<int64> ExpiredRequestIds;
  // 순회 중 맵을 변경하지 않도록 만료된 요청 식별자를 먼저 모은다.
  for (const auto &Pair : PendingRequests)
    if (Pair.Value.ExpireTimeSeconds <= CurrentTime)
      ExpiredRequestIds.Add(Pair.Key);
  for (int64 RequestId : ExpiredRequestIds) {
    FDrPendingRequest PendingRequest;
    if (PendingRequests.RemoveAndCopyValue(RequestId, PendingRequest)) {
      FDrNetResponse Response;
      Response.Result = EDrNetResult::Timeout;
      PendingRequest.Callback.ExecuteIfBound(Response);
    }
  }
  return true;
}
void UDrNetworkSubsystem::Dispatch(const FDrNetPacket &Packet) {
  if (Packet.RequestId) {
    FDrPendingRequest PendingRequest;
    if (PendingRequests.RemoveAndCopyValue(Packet.RequestId, PendingRequest)) {
      FDrNetResponse Response;
      Response.Result = Packet.Result;
      Response.Payload = Packet.Payload;
      PendingRequest.Callback.ExecuteIfBound(Response);
    }
    return;
  }
  OnServerPush.Broadcast(Packet);
}
void UDrNetworkSubsystem::SetConnectionState(EDrConnectionState NewState) {
  if (ConnectionState == NewState)
    return;
  ConnectionState = NewState;
  OnConnectionStateChanged.Broadcast(NewState);
}
