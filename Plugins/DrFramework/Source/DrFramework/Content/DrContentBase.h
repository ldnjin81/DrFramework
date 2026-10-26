// 서버 데이터를 보관하는 콘텐츠 객체의 공통 기반을 선언한다.
#pragma once
#include "CoreMinimal.h"
#include "Network/DrNetTypes.h"
#include "UObject/Object.h"
#include "DrContentBase.generated.h"

class UDrContentSubsystem;

UCLASS(Abstract)
class DRFRAMEWORK_API UDrContentBase : public UObject {
  GENERATED_BODY()

public:
  virtual void Initialize(UDrContentSubsystem *InOwner);
  virtual void HandleServerPacket(const FDrNetPacket &Packet);
  virtual TArray<EDrNetMessageType> GetHandledMessageTypes() const;

protected:
  UPROPERTY() TObjectPtr<UDrContentSubsystem> Owner;
};
