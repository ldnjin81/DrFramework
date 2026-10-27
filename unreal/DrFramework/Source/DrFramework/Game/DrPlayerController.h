#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DrPlayerController.generated.h"
class UDrUIRootLayout;
class UDrHUDWidget;
class UCommonActivatableWidget;
class UInputAction;
class UInputMappingContext;
UCLASS()
class DRFRAMEWORK_API ADrPlayerController : public APlayerController { GENERATED_BODY()
public:
 virtual bool OpenPauseMenu();
 void ClosePauseMenu();
 void TogglePauseMenu();
 void OpenSettings();
 void BackMenu();
 void StartGame();
 void ReturnToTitle();
 UFUNCTION(BlueprintNativeEvent,Category="Dr|Game") void RequestRestart();
 virtual void RequestRestart_Implementation();
 bool IsMenuInput() const { return bMenuInput; }
 bool IsPauseMenuOpen() const { return bPauseOpen; }
 virtual bool IsTitleScreen() const;
 UCommonActivatableWidget* GetSettingsMenu() const { return SettingsMenu; }
 UDrHUDWidget* GetHUDWidget() const { return HUD; }
protected:
 virtual void SetupInputComponent() override;
 virtual void BeginPlay() override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 virtual void OnHUDCreated(UDrHUDWidget* Widget) {}
 virtual void OnMenuChanged(bool bOpen) {}
 virtual bool CanOpenPauseMenu() const { return true; }
 virtual void OnPawnChanged(APawn* PreviousPawn,APawn* NewPawn) {}
 void ApplyInputMode(bool bMenu);
private:
 UFUNCTION() void HandlePawnChanged(APawn* PreviousPawn,APawn* NewPawn);
 UPROPERTY(Transient) TObjectPtr<UDrUIRootLayout> RootLayout;
 UPROPERTY(Transient) TObjectPtr<UDrHUDWidget> HUD;
 UPROPERTY(Transient) TObjectPtr<UCommonActivatableWidget> SettingsMenu;
 UPROPERTY(Transient) TObjectPtr<UInputAction> PauseAction;
 UPROPERTY(Transient) TObjectPtr<UInputMappingContext> PauseContext;
 bool bMenuInput=false;
 bool bPauseOpen=false;
};
