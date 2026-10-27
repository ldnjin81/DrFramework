// 네트워크 서브시스템과 서버 구현 사이의 추상 끝점을 선언한다.
#pragma once
#include "CoreMinimal.h"
#include "Network/DrNetTypes.h"
#include "UObject/Object.h"
#include "DrServerEndpoint.generated.h"

UCLASS(Abstract)
class DRFRAMEWORK_API UDrServerEndpoint : public UObject {
  GENERATED_BODY()
public:
  virtual void SubmitFromClient(const FDrNetPacket &Packet)
      PURE_VIRTUAL(UDrServerEndpoint::SubmitFromClient, );
  virtual void OpenConnection()
      PURE_VIRTUAL(UDrServerEndpoint::OpenConnection, );
  virtual void CloseConnection()
      PURE_VIRTUAL(UDrServerEndpoint::CloseConnection, );
  FDrServerPacketDelegate OnPacketToClient;
  FDrConnectionStateDelegate OnEndpointConnectionChanged;
};
