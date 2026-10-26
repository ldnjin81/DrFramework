// 게임 인스턴스 범위의 요청·응답 네트워크 계층을 선언한다.
#pragma once
#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "Network/DrNetTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DrNetworkSubsystem.generated.h"

class UDrServerEndpoint;

/** 응답 대기 중인 콜백과 만료 시각이다. */
USTRUCT()
struct FDrPendingRequest {
  GENERATED_BODY()
  FDrResponseDelegate Callback;
  double ExpireTimeSeconds = 0.;
};
/** 클라이언트 수신 시각까지 대기하는 패킷이다. */
USTRUCT()
struct FDrQueuedClientPacket {
  GENERATED_BODY()
  UPROPERTY()
  FDrNetPacket Packet;
  UPROPERTY()
  double ArrivalTime = 0.;
};
/** 서버 끝점 연결과 요청 수명을 관리한다. */
UCLASS()
class DRFRAMEWORK_API UDrNetworkSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()
public:
  virtual void Initialize(FSubsystemCollectionBase &Collection) override;
  virtual void Deinitialize() override;
  void Connect();
  UDrServerEndpoint* GetServerEndpoint() const { return ServerEndpoint; }
  void SetSessionToken(const FString& Value) { SessionToken=Value; }
  void Disconnect();
  void SetLatency(float BaseSeconds, float JitterSeconds, float FailureRate);
  int64 SendRequest(FName Type, const FInstancedStruct &Payload,
                    FDrResponseDelegate OnResponse, float TimeoutSeconds = 5.f);
  template <typename PayloadType>
  int64 SendRequest(FName Type, const PayloadType &Payload,
                    FDrResponseDelegate OnResponse,
                    float TimeoutSeconds = 5.f) {
    return SendRequest(Type, FInstancedStruct::Make(Payload), OnResponse,
                       TimeoutSeconds);
  }
  EDrConnectionState GetConnectionState() const { return ConnectionState; }
  const FString &GetSessionToken() const { return SessionToken; }
  FDrConnectionStateDelegate OnConnectionStateChanged;
  FDrServerPacketDelegate OnServerPush;
  UPROPERTY(EditAnywhere, Category = "Dr|Network")
  FDrLatencyProfile ClientLatency;

private:
  void HandlePacketToClient(const FDrNetPacket &Packet);
  void HandleEndpointConnectionChanged(EDrConnectionState NewState);
  bool Tick(float DeltaTime);
  void Dispatch(const FDrNetPacket &Packet);
  void SetConnectionState(EDrConnectionState NewState);
  UPROPERTY() TObjectPtr<UDrServerEndpoint> ServerEndpoint;
  UPROPERTY(EditAnywhere, Category = "Dr|Network")
  TSubclassOf<UDrServerEndpoint> ServerEndpointClass;
  TArray<FDrQueuedClientPacket> ReceiveQueue;
  TMap<int64, FDrPendingRequest> PendingRequests;
  FString SessionToken;
  int64 NextRequestId = 1;
  double LastReceiveArrivalTime = 0.;
  EDrConnectionState ConnectionState = EDrConnectionState::Disconnected;
  FTSTicker::FDelegateHandle TickHandle;
};
