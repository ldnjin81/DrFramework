#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/World.h"
#include "DrFlowSettings.generated.h"

/**
 * 게임 흐름 설정(타이틀 맵·게임 맵). 0.2.0의 UDrUISettings에서 맵만 남긴 것이다.
 * 메뉴·HUD 클래스와 UI 입력은 0.3.0부터 DrUIKit(UDrUIKitSettings '흐름'·'입력')이 맡는다.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "DrFramework 흐름"))
class DRFRAMEWORK_API UDrFlowSettings : public UDeveloperSettings {
  GENERATED_BODY()
public:
  UPROPERTY(Config, EditAnywhere, Category = "화면") TSoftObjectPtr<UWorld> TitleMap;
  UPROPERTY(Config, EditAnywhere, Category = "화면") TSoftObjectPtr<UWorld> GameMap;
};
