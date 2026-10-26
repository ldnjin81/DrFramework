#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DrPlayerController.generated.h"

class UDrCombatHUDViewModel;
class UDrCombatHUDWidget;
class UDrUIRootLayout;
class UDrSettingsMenuWidget;
class UInputAction;
class UInputMappingContext;

/** 로컬 플레이어마다 표시 모델과 HUD를 소유한다. */
UCLASS()
class DRFRAMEWORK_API ADrPlayerController : public APlayerController {
  GENERATED_BODY()
public:
  bool OpenPauseMenu();
  void ClosePauseMenu();
  void TogglePauseMenu();
  void OpenSettings();
  void BackMenu();
  void StartGame();
  void ReturnToTitle();
  void RestartCheckpoint();
  bool IsMenuInput() const { return bMenuInput; }
  bool IsPauseMenuOpen() const { return bPauseOpen; }
  bool IsTitleScreen() const;
  UDrSettingsMenuWidget* GetSettingsMenu() const { return SettingsMenu; }
  UDrCombatHUDWidget* GetCombatHUD() const { return CombatHUD; }
protected:
  virtual void SetupInputComponent() override;
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
  void ApplyInputMode(bool bMenu);
  UFUNCTION() void HandlePawnChanged(APawn *OldPawn, APawn *NewPawn);
  UPROPERTY(Transient) TObjectPtr<UDrCombatHUDViewModel> HUDViewModel;
  UPROPERTY(Transient) TObjectPtr<UDrCombatHUDWidget> CombatHUD;
  UPROPERTY(Transient) TObjectPtr<UDrUIRootLayout> RootLayout;
  UPROPERTY(Transient) TObjectPtr<UDrSettingsMenuWidget> SettingsMenu;
  UPROPERTY(Transient) TObjectPtr<UInputAction> PauseAction;
  UPROPERTY(Transient) TObjectPtr<UInputMappingContext> PauseContext;
  bool bMenuInput = false;
  bool bPauseOpen = false;
};
