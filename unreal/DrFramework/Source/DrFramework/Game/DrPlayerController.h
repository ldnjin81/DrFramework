#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DrPlayerController.generated.h"

/**
 * 게임 흐름 컨트롤러: 타이틀 판정·게임 시작·타이틀 복귀·재시작. UI(화면 층·메뉴·HUD·일시정지 메뉴·입력 모드)는 0.3.0부터
 * DrUIKit의 UDrUIFlowSubsystem이 맡는다. 화면이 정해지면 OnEnterTitle·OnEnterGame이 불리므로 게임은 거기서 UI 흐름을 시작한다
 * (예: UDrUIFlowSubsystem::Get(this)->EnterTitle()). 이 플러그인은 다른 UI 플러그인에 의존하지 않는다.
 */
UCLASS()
class DRFRAMEWORK_API ADrPlayerController : public APlayerController {
  GENERATED_BODY()
public:
  /** 타이틀 화면에서 게임 맵으로. */
  void StartGame();
  /** 일시정지를 풀고 타이틀 맵으로. */
  void ReturnToTitle();
  UFUNCTION(BlueprintNativeEvent, Category = "Dr|Game") void RequestRestart();
  virtual void RequestRestart_Implementation();
  /** 지금 맵이 설정의 타이틀 맵인지. */
  virtual bool IsTitleScreen() const;
protected:
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type Reason) override;
  /** 로컬 컨트롤러가 타이틀 맵에서 시작할 때. 게임은 여기서 타이틀 UI를 연다. */
  virtual void OnEnterTitle() { BP_OnEnterTitle(); }
  /** 로컬 컨트롤러가 게임 맵에서 시작할 때. 게임은 여기서 HUD·게임 입력을 켠다. */
  virtual void OnEnterGame() { BP_OnEnterGame(); }
  UFUNCTION(BlueprintImplementableEvent, Category = "Dr|Game", meta = (DisplayName = "On Enter Title")) void BP_OnEnterTitle();
  UFUNCTION(BlueprintImplementableEvent, Category = "Dr|Game", meta = (DisplayName = "On Enter Game")) void BP_OnEnterGame();
  virtual void OnPawnChanged(APawn* PreviousPawn, APawn* NewPawn) {}
private:
  UFUNCTION() void HandlePawnChanged(APawn* PreviousPawn, APawn* NewPawn);
};
