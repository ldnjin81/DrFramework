// 클라이언트와 서버 끝점이 공유하는 네트워크 메시지 타입을 선언한다.
#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "DrNetTypes.generated.h"

/** 요청과 서버 푸시의 메시지 종류이다. */
UENUM()
enum class EDrNetMessageType : uint8 {
  None,
  Ping,
  Login,
  RequestPlayerProfile,
  ServerPush_ProfileUpdated
};
/** 네트워크 끝점의 연결 상태이다. */
UENUM()
enum class EDrConnectionState : uint8 {
  Disconnected,
  Connecting,
  Connected,
  Reconnecting
};
/** 요청 처리 결과이다. */
UENUM()
enum class EDrNetResult : uint8 {
  Ok,
  Timeout,
  Error,
  NotConnected,
  InvalidRequest
};
/** 메시지 종류, 요청 식별자, 페이로드와 결과를 담는 전송 단위이다. */
USTRUCT()
struct FDrNetPacket {
  GENERATED_BODY()

  UPROPERTY()
  EDrNetMessageType Type = EDrNetMessageType::None;
  UPROPERTY()
  int64 RequestId = 0;
  UPROPERTY()
  FInstancedStruct Payload;
  UPROPERTY()
  double SendTimeSeconds = 0.;
  UPROPERTY()
  EDrNetResult Result = EDrNetResult::Ok;
};
/** 요청 콜백으로 전달되는 결과와 페이로드이다. */
USTRUCT()
struct FDrNetResponse {
  GENERATED_BODY()

  UPROPERTY()
  EDrNetResult Result = EDrNetResult::Error;
  UPROPERTY()
  FInstancedStruct Payload;
};
/** 로컬 통신의 지연, 흔들림, 실패율을 재현하는 설정이다. */
USTRUCT()
struct FDrLatencyProfile {
  GENERATED_BODY()
  UPROPERTY(EditAnywhere, Category = "Dr|Network")
  bool bEnabled = true;
  UPROPERTY(EditAnywhere, Category = "Dr|Network")
  float BaseLatencySeconds = .08f;
  UPROPERTY(EditAnywhere, Category = "Dr|Network")
  float JitterSeconds = .03f;
  UPROPERTY(EditAnywhere, Category = "Dr|Network",
            meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float FailureRate = 0.f;
  float RollLatency() const {
    return bEnabled
               ? FMath::Max(0.f, BaseLatencySeconds +
                                     (JitterSeconds > 0.f
                                          ? FMath::FRandRange(-JitterSeconds,
                                                              JitterSeconds)
                                          : 0.f))
               : 0.f;
  }
  bool RollFailure() const {
    return bEnabled && FailureRate > 0.f &&
           FMath::FRand() < FMath::Clamp(FailureRate, 0.f, 1.f);
  }
};
/** 연결 상태를 확인하기 위한 왕복 메시지이다. */
USTRUCT()
struct FDrPingPayload {
  GENERATED_BODY()
  UPROPERTY()
  FString Message;
};
/** 로그인에 사용할 사용자 식별자이다. */
USTRUCT()
struct FDrLoginRequest {
  GENERATED_BODY()
  UPROPERTY()
  FString UserId;
};
/** 로그인 성공 시 발급되는 세션 토큰이다. */
USTRUCT()
struct FDrLoginResponse {
  GENERATED_BODY()
  UPROPERTY()
  FString SessionToken;
};
/** 별도 값이 필요 없는 요청에 사용하는 빈 페이로드이다. */
USTRUCT()
struct FDrEmptyPayload {
  GENERATED_BODY()
};
DECLARE_DELEGATE_OneParam(FDrResponseDelegate, const FDrNetResponse &);
DECLARE_MULTICAST_DELEGATE_OneParam(FDrConnectionStateDelegate,
                                    EDrConnectionState);
DECLARE_MULTICAST_DELEGATE_OneParam(FDrServerPacketDelegate,
                                    const FDrNetPacket &);
