#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UI/DrMenuWidget.h"
#include "UI/DrHUDWidget.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/World.h"
#include "DrUISettings.generated.h"
UCLASS(Config=Game,DefaultConfig,meta=(DisplayName="DrFramework UI"))
class DRFRAMEWORK_API UDrUISettings : public UDeveloperSettings { GENERATED_BODY()
public:
 UPROPERTY(Config,EditAnywhere,Category="메뉴") TSoftClassPtr<UDrMenuWidget> TitleMenuClass;
 UPROPERTY(Config,EditAnywhere,Category="메뉴") TSoftClassPtr<UDrMenuWidget> PauseMenuClass;
 UPROPERTY(Config,EditAnywhere,Category="메뉴") TSoftClassPtr<UDrMenuWidget> SettingsMenuClass;
 UPROPERTY(Config,EditAnywhere,Category="HUD") TSoftClassPtr<UDrHUDWidget> HUDClass;
 UPROPERTY(Config,EditAnywhere,Category="입력") TSoftObjectPtr<UInputMappingContext> UIInputContext;
 UPROPERTY(Config,EditAnywhere,Category="입력") TSoftObjectPtr<UInputAction> PauseAction;
 UPROPERTY(Config,EditAnywhere,Category="입력") TSoftObjectPtr<UInputMappingContext> MenuInputContext;
 UPROPERTY(Config,EditAnywhere,Category="화면") TSoftObjectPtr<UWorld> TitleMap;
 UPROPERTY(Config,EditAnywhere,Category="화면") TSoftObjectPtr<UWorld> GameMap;
};
