// 콘텐츠 객체의 기본 초기화와 기본 패킷 처리를 구현한다.
#include "Content/DrContentBase.h"
void UDrContentBase::Initialize(UDrContentSubsystem *InOwner) {
  Owner = InOwner;
}
void UDrContentBase::HandleServerPacket(const FDrNetPacket &) {}
TArray<FName> UDrContentBase::GetHandledMessageTypes() const {
  return {};
}
