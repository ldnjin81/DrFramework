#include "Game/DrPlayerController.h"
#include "Game/DrFlowSettings.h"
#include "Kismet/GameplayStatics.h"

bool ADrPlayerController::IsTitleScreen() const {
  const auto& Map = GetDefault<UDrFlowSettings>()->TitleMap;
  return GetWorld() && !Map.IsNull() && UGameplayStatics::GetCurrentLevelName(this, true) == FPackageName::GetShortName(Map.GetLongPackageName());
}

void ADrPlayerController::StartGame() {
  if (!IsTitleScreen()) return;
  const auto& Map = GetDefault<UDrFlowSettings>()->GameMap;
  if (!Map.IsNull()) { SetPause(false); UGameplayStatics::OpenLevel(this, *Map.GetLongPackageName()); }
}

void ADrPlayerController::ReturnToTitle() {
  SetPause(false);
  const auto& Map = GetDefault<UDrFlowSettings>()->TitleMap;
  if (!Map.IsNull()) UGameplayStatics::OpenLevel(this, *Map.GetLongPackageName());
}

void ADrPlayerController::RequestRestart_Implementation() { SetPause(false); }

void ADrPlayerController::BeginPlay() {
  Super::BeginPlay();
  if (!IsLocalController()) return;
  OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::HandlePawnChanged);
  if (IsTitleScreen()) OnEnterTitle();
  else OnEnterGame();
  HandlePawnChanged(nullptr, GetPawn());
}

void ADrPlayerController::HandlePawnChanged(APawn* PreviousPawn, APawn* NewPawn) { OnPawnChanged(PreviousPawn, NewPawn); }

void ADrPlayerController::EndPlay(const EEndPlayReason::Type Reason) {
  OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::HandlePawnChanged);
  Super::EndPlay(Reason);
}
