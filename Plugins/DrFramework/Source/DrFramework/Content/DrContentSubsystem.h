// 게임 인스턴스 범위의 콘텐츠 캐시 관리자를 선언한다.
#pragma once
#include "CoreMinimal.h"
#include "Network/DrNetTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DrContentSubsystem.generated.h"
class UDrContentBase;
class UDrPlayerProfileContent;
UCLASS()
class DRFRAMEWORK_API UDrContentSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()
public:
  virtual void Initialize(FSubsystemCollectionBase &Collection) override;
  virtual void Deinitialize() override;
  UDrPlayerProfileContent *GetPlayerProfile() const { return PlayerProfile; }

private:
  void RegisterContent(UDrContentBase *Content);
  void HandleServerPush(const FDrNetPacket &Packet);
  UPROPERTY() TObjectPtr<UDrPlayerProfileContent> PlayerProfile;
  UPROPERTY() TArray<TObjectPtr<UDrContentBase>> Contents;
};
