// 가짜 서버의 처리기 등록 확장 지점(게임 정의 메시지 ID → 처리기)을 검증한다.
#if WITH_DEV_AUTOMATION_TESTS
#include "Backend/DrFakeServer.h"
#include "Misc/AutomationTest.h"
#include "Network/DrNetTypes.h"

namespace DrBackendTests {
/** 지연 없이 요청 하나를 보내고 서버가 클라이언트로 보낸 응답을 돌려준다. */
FDrNetPacket RoundTrip(UDrFakeServer *Server, FName Type, const FInstancedStruct &Payload = FInstancedStruct()) {
  FDrNetPacket Received;
  Received.Result = EDrNetResult::Error;
  const FDelegateHandle Handle = Server->OnPacketToClient.AddLambda(
      [&Received](const FDrNetPacket &Packet) { Received = Packet; });
  FDrNetPacket Request;
  Request.Type = Type;
  Request.RequestId = 7;
  Request.Payload = Payload;
  Server->SubmitFromClient(Request);
  Server->Test_Tick(0.f);
  Server->OnPacketToClient.Remove(Handle);
  return Received;
}
} // namespace DrBackendTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrFakeServerHandlerTest,
                                 "DrFramework.Framework.Backend.RegisteredHandlerResponds",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDrFakeServerHandlerTest::RunTest(const FString &) {
  UDrFakeServer *Server = NewObject<UDrFakeServer>();
  Server->LatencyProfile.bEnabled = false;
  Server->OpenConnection();
  int32 Calls = 0;
  Server->RegisterHandler(TEXT("Echo"), [&Calls](const FDrNetPacket &Request) {
    ++Calls;
    FDrNetResponse Response;
    Response.Result = EDrNetResult::Ok;
    FDrPingPayload Payload;
    Payload.Message = Request.Payload.Get<FDrPingPayload>().Message + TEXT("!");
    Response.Payload = FInstancedStruct::Make(Payload);
    return Response;
  });
  FDrPingPayload Sent;
  Sent.Message = TEXT("hi");
  const FDrNetPacket Reply = DrBackendTests::RoundTrip(Server, TEXT("Echo"), FInstancedStruct::Make(Sent));
  TestEqual(TEXT("처리기 1회 호출"), Calls, 1);
  TestEqual(TEXT("처리기 결과"), Reply.Result, EDrNetResult::Ok);
  TestEqual(TEXT("요청 Id 보존"), Reply.RequestId, int64(7));
  TestTrue(TEXT("처리기 페이로드"), Reply.Payload.GetPtr<FDrPingPayload>() &&
                                     Reply.Payload.Get<FDrPingPayload>().Message == TEXT("hi!"));
  Server->CloseConnection();
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrFakeServerDefaultsTest,
                                 "DrFramework.Framework.Backend.PingAndUnknownId",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDrFakeServerDefaultsTest::RunTest(const FString &) {
  UDrFakeServer *Server = NewObject<UDrFakeServer>();
  Server->LatencyProfile.bEnabled = false;
  Server->OpenConnection();
  const FDrNetPacket Pong = DrBackendTests::RoundTrip(Server, TEXT("Ping"));
  TestEqual(TEXT("기본 Ping 응답"), Pong.Result, EDrNetResult::Ok);
  TestTrue(TEXT("pong 페이로드"), Pong.Payload.GetPtr<FDrPingPayload>() &&
                                   Pong.Payload.Get<FDrPingPayload>().Message == TEXT("pong"));
  const FDrNetPacket Unknown = DrBackendTests::RoundTrip(Server, TEXT("NotRegistered"));
  TestEqual(TEXT("등록 안 된 Id는 InvalidRequest"), Unknown.Result, EDrNetResult::InvalidRequest);
  Server->CloseConnection();
  return true;
}
#endif
