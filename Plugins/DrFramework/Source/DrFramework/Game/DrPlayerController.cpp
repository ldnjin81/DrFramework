#include "Game/DrPlayerController.h"
#include "TableData/DrGameAssets.h"

#include "Characters/DrCharacter.h"
#include "Engine/World.h"
#include "UI/DrCombatHUDViewModel.h"
#include "UI/DrCombatHUDWidget.h"
#include "Misc/App.h"
#include "UObject/SoftObjectPath.h"
#include "UI/DrMenuWidget.h"
#include "UI/DrTitleMenuWidget.h"
#include "UI/DrPauseMenuWidget.h"
#include "UI/DrSettingsMenuWidget.h"
#include "UI/DrUIRootLayout.h"
#include "Game/DrTitleGameMode.h"
#include "Abilities/DrAttributeSet.h"
#include "Encounter/DrEncounterSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"

namespace {
UClass* MenuClass(const TCHAR* Path, UClass* Fallback) {
  UClass* Loaded = FSoftClassPath(Path).TryLoadClass<UDrMenuWidget>();
  return Loaded ? Loaded : Fallback;
}
}

void ADrPlayerController::SetupInputComponent() {
  Super::SetupInputComponent();
  PauseAction = LoadObject<UInputAction>(nullptr, *DrGameAssets::Path(TEXT("IA_Pause")).ToString());
  PauseContext = LoadObject<UInputMappingContext>(nullptr, *DrGameAssets::Path(TEXT("IMC_Menu")).ToString());
  if (auto* Enhanced = Cast<UEnhancedInputComponent>(InputComponent)) {
    if (PauseAction) Enhanced->BindAction(PauseAction, ETriggerEvent::Started, this, &ThisClass::TogglePauseMenu);
  }
  if (GetLocalPlayer() && PauseContext) {
    if (auto* Input = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
      Input->AddMappingContext(PauseContext, 100);
  }
}
bool ADrPlayerController::IsTitleScreen() const {
  return GetWorld() && GetWorld()->GetAuthGameMode<ADrTitleGameMode>() != nullptr;
}
void ADrPlayerController::ApplyInputMode(bool bMenu) {
  bMenuInput = bMenu;
  bShowMouseCursor = bMenu;
  if (!GetLocalPlayer()) return;
  if (bMenu) {
    FInputModeUIOnly Mode;
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(Mode);
  } else SetInputMode(FInputModeGameOnly());
}
bool ADrPlayerController::OpenPauseMenu() {
  if (IsTitleScreen() || bPauseOpen) return false;
  auto* DrPawn = Cast<ADrCharacter>(GetPawn());
  auto* Encounter = GetWorld()->GetSubsystem<UDrEncounterSubsystem>();
  if (!DrPawn || !DrPawn->GetAttributeSet() || DrPawn->GetAttributeSet()->GetHealth() <= 0.f ||
      (Encounter && Encounter->GetState() == EDrEncounterState::PlayerDead)) return false;
  if (!SetPause(true)) return false;
  bPauseOpen = true;
  if (RootLayout) RootLayout->PushMenu(MenuClass(*DrGameAssets::Path(TEXT("WBP_PauseMenu")).ToString(), UDrPauseMenuWidget::StaticClass()));
  ApplyInputMode(true);
  return true;
}
void ADrPlayerController::ClosePauseMenu() {
  if (!bPauseOpen) return;
  SettingsMenu = nullptr;
  if (RootLayout) RootLayout->ClearMenus();
  bPauseOpen = false;
  SetPause(false);
  ApplyInputMode(false);
}
void ADrPlayerController::TogglePauseMenu() {
  if (bPauseOpen) BackMenu(); else OpenPauseMenu();
}
void ADrPlayerController::OpenSettings() {
  if (SettingsMenu || (!bPauseOpen && !IsTitleScreen()) || !RootLayout) return;
  SettingsMenu = Cast<UDrSettingsMenuWidget>(RootLayout->PushMenu(MenuClass(*DrGameAssets::Path(TEXT("WBP_Settings")).ToString(), UDrSettingsMenuWidget::StaticClass())));
  ApplyInputMode(true);
}
void ADrPlayerController::BackMenu() {
  if (SettingsMenu) {
    if (RootLayout) RootLayout->PopMenu();
    SettingsMenu = nullptr;
  } else if (!IsTitleScreen()) ClosePauseMenu();
}
void ADrPlayerController::StartGame() {
  if (!IsTitleScreen()) return;
  SetPause(false);
  UGameplayStatics::OpenLevel(this, *DrGameAssets::Path(TEXT("L_BossArena")).GetLongPackageName());
}
void ADrPlayerController::ReturnToTitle() {
  ClosePauseMenu();
  UGameplayStatics::OpenLevel(this, *DrGameAssets::Path(TEXT("L_Title")).GetLongPackageName());
}
void ADrPlayerController::RestartCheckpoint() {
  ClosePauseMenu();
  if (auto* Encounter = GetWorld()->GetSubsystem<UDrEncounterSubsystem>()) Encounter->RestartCheckpoint();
}

void ADrPlayerController::BeginPlay() {
  Super::BeginPlay();
  if (!IsLocalController()) return;
  if (FApp::CanEverRender() && GetLocalPlayer()) {
    RootLayout = CreateWidget<UDrUIRootLayout>(this);
    RootLayout->AddToPlayerScreen();
  }
  if (IsTitleScreen()) {
    if (RootLayout) RootLayout->PushMenu(MenuClass(*DrGameAssets::Path(TEXT("WBP_TitleMenu")).ToString(), UDrTitleMenuWidget::StaticClass()));
    ApplyInputMode(true);
    return;
  }
  // 타이틀의 UI 전용 모드가 남긴 뷰포트 입력 무시는 레벨을 넘어 유지되므로 게임 레벨에서 되돌린다.
  ApplyInputMode(false);
  HUDViewModel = NewObject<UDrCombatHUDViewModel>(this);
  HUDViewModel->Initialize(GetWorld()->GetSubsystem<UDrEncounterSubsystem>());
  OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::HandlePawnChanged);
  HandlePawnChanged(nullptr, GetPawn());
  // 전용 서버와 nullrhi 자동화에는 표시 모델만 만든다.
  if (!FApp::CanEverRender() || !GetLocalPlayer()) return;
  UClass *HUDClass = FSoftClassPath(*DrGameAssets::Path(TEXT("WBP_CombatHUD")).ToString()).TryLoadClass<UDrCombatHUDWidget>();
  CombatHUD = RootLayout ? Cast<UDrCombatHUDWidget>(RootLayout->PushGame(HUDClass ? HUDClass : UDrCombatHUDWidget::StaticClass())) : nullptr;
  if (CombatHUD) {
    CombatHUD->SetViewModel(HUDViewModel);
  }
}

void ADrPlayerController::HandlePawnChanged(APawn *, APawn *NewPawn) {
  if (HUDViewModel) HUDViewModel->BindPlayer(Cast<ADrCharacter>(NewPawn));
}

void ADrPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::HandlePawnChanged);
  if (CombatHUD) CombatHUD->RemoveFromParent();
  if (RootLayout) RootLayout->RemoveFromParent();
  if (GetLocalPlayer() && PauseContext) {
    if (auto* Input = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
      Input->RemoveMappingContext(PauseContext);
  }
  if (HUDViewModel) HUDViewModel->Shutdown();
  Super::EndPlay(EndPlayReason);
}
