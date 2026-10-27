// 네트워크 흐름을 로컬에서 재현하는 가짜 서버 끝점을 선언한다.
#pragma once
#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "Network/DrServerEndpoint.h"
#include "DrFakeServer.generated.h"

USTRUCT()
struct FDrQueuedPacket {
  GENERATED_BODY()

  UPROPERTY()
  FDrNetPacket Packet;

  UPROPERTY()
  double ArrivalTime = 0.;

  UPROPERTY()
  bool bForceError = false;
};

UCLASS()
class DRFRAMEWORK_API UDrFakeServer : public UDrServerEndpoint {
  GENERATED_BODY()
public:
  using FHandler = TFunction<FDrNetResponse(const FDrNetPacket&)>;
  void RegisterHandler(FName Id, FHandler Handler) { Handlers.Add(Id, MoveTemp(Handler)); }
  virtual void SubmitFromClient(const FDrNetPacket &Packet) override;
  virtual void OpenConnection() override;
  virtual void CloseConnection() override;
  UPROPERTY(EditAnywhere, Category = "Dr|Network")
  FDrLatencyProfile LatencyProfile;
#if WITH_AUTOMATION_TESTS
  // 자동화 테스트에서 지연 큐만 관찰하기 위한 최소 접근자이다.
  const TArray<FDrQueuedPacket> &Test_GetReceiveQueue() const {
    return ReceiveQueue;
  }
  TArray<FDrQueuedPacket> &Test_GetMutableReceiveQueue() {
    return ReceiveQueue;
  }
  bool Test_Tick(float DeltaTime) { return Tick(DeltaTime); }
#endif
private:
  bool Tick(float DeltaTime);
  void ProcessRequest(const FDrQueuedPacket &QueuedPacket);
  FDrNetPacket MakeResponse(const FDrNetPacket &Request, EDrNetResult Result,
                            const FInstancedStruct &Payload) const;
  UPROPERTY() TMap<FString, FInstancedStruct> Store;
  UPROPERTY() TArray<FDrQueuedPacket> ReceiveQueue;
  bool bOnline = false;
  double LastReceiveArrivalTime = 0.;
  TMap<FName, FHandler> Handlers;
  FTSTicker::FDelegateHandle TickHandle;
};
