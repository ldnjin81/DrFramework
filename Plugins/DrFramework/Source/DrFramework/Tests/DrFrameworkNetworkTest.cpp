// 네트워크 페이로드와 가짜 서버 큐를 검증하는 자동화 테스트이다.
#if WITH_DEV_AUTOMATION_TESTS
#include "Backend/DrFakeServer.h"
#include "Misc/AutomationTest.h"
#include "Network/DrNetTypes.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrLatencyProfileTest,
                                 "DrFramework.Framework.Network.LatencyProfile",
                                 EAutomationTestFlags::EditorContext |
                                     EAutomationTestFlags::EngineFilter)
bool FDrLatencyProfileTest::RunTest(const FString &) {
  FDrLatencyProfile Profile;
  // 지연 값은 구조체 필드명에 맞춰 초 단위로 검증한다.
  Profile.bEnabled = false;
  TestEqual(TEXT("비활성 지연"), Profile.RollLatency(), 0.f);
  Profile.bEnabled = true;
  Profile.BaseLatencySeconds = .1f;
  Profile.JitterSeconds = 0;
  TestEqual(TEXT("고정 지연"), Profile.RollLatency(), .1f);
  return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrPacketPayloadTest,
                                 "DrFramework.Framework.Network.PacketPayload",
                                 EAutomationTestFlags::EditorContext |
                                     EAutomationTestFlags::EngineFilter)
bool FDrPacketPayloadTest::RunTest(const FString &) {
  // 프로젝트 구조체를 FInstancedStruct에 담았다가 같은 타입과 값으로 꺼낸다.
  FDrPingPayload PingPayload;
  PingPayload.Message = TEXT("ping");
  FDrNetPacket Packet;
  Packet.Payload = FInstancedStruct::Make(PingPayload);
  TestEqual(TEXT("페이로드"), Packet.Payload.Get<FDrPingPayload>().Message,
            FString(TEXT("ping")));
  return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDrFakeServerQueueTest,
    "DrFramework.Framework.Network.FakeServerReceiveQueue",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDrFakeServerQueueTest::RunTest(const FString &) {
  UDrFakeServer *Server = NewObject<UDrFakeServer>();
  Server->LatencyProfile.bEnabled = false;
  Server->OpenConnection();
  FDrNetPacket Packet;
  Packet.Type = FName(TEXT("Ping"));
  Server->SubmitFromClient(Packet);
  TestEqual(TEXT("큐 수"), Server->Test_GetReceiveQueue().Num(), 1);
  Server->CloseConnection();
  return true;
}
#endif
