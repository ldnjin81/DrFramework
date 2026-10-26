// 플레이어 프로필 요청, 응답, 갱신 처리를 구현한다.
#include "Content/DrPlayerProfileContent.h"
#include "Content/DrContentSubsystem.h"
#include "Network/DrNetworkSubsystem.h"
#include "DrFramework.h"
void UDrPlayerProfileContent::HandleServerPacket(const FDrNetPacket &Packet) {
  if (Packet.Type == EDrNetMessageType::ServerPush_ProfileUpdated &&
      Packet.Result == EDrNetResult::Ok)
    if (const FDrPlayerProfileDataSet *ProfileDataSet =
            Packet.Payload.GetPtr<FDrPlayerProfileDataSet>())
      ApplyProfileDataSet(*ProfileDataSet);
}
TArray<EDrNetMessageType>
UDrPlayerProfileContent::GetHandledMessageTypes() const {
  return {EDrNetMessageType::ServerPush_ProfileUpdated};
}
void UDrPlayerProfileContent::RequestProfile() {
  UDrNetworkSubsystem *NetworkSubsystem =
      Owner ? Owner->GetGameInstance()->GetSubsystem<UDrNetworkSubsystem>()
            : nullptr;
  if (!NetworkSubsystem) {
    UE_LOG(LogDr, Error,
           TEXT("[Content] 네트워크 없이 프로필을 요청할 수 없습니다."));
    return;
  }
  NetworkSubsystem->SendRequest(
      EDrNetMessageType::RequestPlayerProfile, FDrEmptyPayload(),
      FDrResponseDelegate::CreateUObject(
          this, &UDrPlayerProfileContent::HandleProfileResponse));
}
void UDrPlayerProfileContent::HandleProfileResponse(
    const FDrNetResponse &Response) {
  if (Response.Result == EDrNetResult::Ok)
    if (const FDrPlayerProfileDataSet *ProfileDataSet =
            Response.Payload.GetPtr<FDrPlayerProfileDataSet>())
      ApplyProfileDataSet(*ProfileDataSet);
}
void UDrPlayerProfileContent::ApplyProfileDataSet(
    const FDrPlayerProfileDataSet &InDataSet) {
  DataSet = InDataSet;
  OnProfileChanged.Broadcast(DataSet);
}
