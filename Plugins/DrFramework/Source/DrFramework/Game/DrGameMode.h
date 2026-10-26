#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DrGameMode.generated.h"

/** 프로젝트 캐릭터를 기본 폰으로 사용하는 게임 모드이다. */
UCLASS()
class DRFRAMEWORK_API ADrGameMode : public AGameModeBase {
  GENERATED_BODY()
public:
  ADrGameMode();
  virtual void InitGame(const FString &MapName, const FString &Options,
                        FString &ErrorMessage) override;
};
