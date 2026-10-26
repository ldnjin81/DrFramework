// 콘텐츠 객체의 수명과 서버 푸시 전달을 관리한다.
#include "Content/DrContentSubsystem.h"
#include "Content/DrContentBase.h"
#include "Content/DrPlayerProfileContent.h"
#include "Network/DrNetworkSubsystem.h"
void UDrContentSubsystem::Initialize(FSubsystemCollectionBase &Collection) {
  Collection.InitializeDependency(UDrNetworkSubsystem::StaticClass());
  Super::Initialize(Collection);
  PlayerProfile = NewObject<UDrPlayerProfileContent>(this);
  RegisterContent(PlayerProfile);
  GetGameInstance()
      ->GetSubsystem<UDrNetworkSubsystem>()
      ->OnServerPush.AddUObject(this, &UDrContentSubsystem::HandleServerPush);
}
void UDrContentSubsystem::Deinitialize() {
  if (UDrNetworkSubsystem *NetworkSubsystem =
          GetGameInstance()->GetSubsystem<UDrNetworkSubsystem>())
    NetworkSubsystem->OnServerPush.RemoveAll(this);
  Contents.Empty();
  PlayerProfile = nullptr;
  Super::Deinitialize();
}
void UDrContentSubsystem::RegisterContent(UDrContentBase *Content) {
  if (Content) {
    Content->Initialize(this);
    Contents.Add(Content);
  }
}
void UDrContentSubsystem::HandleServerPush(const FDrNetPacket &Packet) {
  for (UDrContentBase *Content : Contents)
    if (Content && Content->GetHandledMessageTypes().Contains(Packet.Type))
      Content->HandleServerPacket(Packet);
}
