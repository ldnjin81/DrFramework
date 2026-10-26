// 게임 인스턴스 범위의 콘텐츠 캐시 관리자를 선언한다.
#pragma once
#include "CoreMinimal.h"
#include "Network/DrNetTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DrContentSubsystem.generated.h"
class UDrContentBase;
UCLASS()
class DRFRAMEWORK_API UDrContentSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()
public:
  virtual void Initialize(FSubsystemCollectionBase &Collection) override;
  virtual void Deinitialize() override;
  void RegisterContent(UDrContentBase* Content);
  template<class T> T* FindContent() const { for(const auto& Item : Contents) if(auto* Found=Cast<T>(Item.Get())) return Found; return nullptr; }

private:
  void HandleServerPush(const FDrNetPacket &Packet);
  UPROPERTY() TArray<TObjectPtr<UDrContentBase>> Contents;
};
