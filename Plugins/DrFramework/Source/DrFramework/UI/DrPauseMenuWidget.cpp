#include "UI/DrPauseMenuWidget.h"

#include "UI/DrGlassButton.h"
#include "Game/DrPlayerController.h"

void UDrPauseMenuWidget::BuildMenu(UVerticalBox* Contents) {
  AddHeading(Contents, TEXT("Paused"));
  AddButton(Contents, TEXT("Continue"))->OnClicked().AddWeakLambda(this, [this] {
    if (auto* PC = GetOwningPlayer<ADrPlayerController>()) PC->ClosePauseMenu();
  });
  AddButton(Contents, TEXT("Restart"))->OnClicked().AddWeakLambda(this, [this] {
    if (auto* PC = GetOwningPlayer<ADrPlayerController>()) PC->RestartCheckpoint();
  });
  AddButton(Contents, TEXT("Settings"))->OnClicked().AddWeakLambda(this, [this] {
    if (auto* PC = GetOwningPlayer<ADrPlayerController>()) PC->OpenSettings();
  });
  AddButton(Contents, TEXT("ToTitle"))->OnClicked().AddWeakLambda(this, [this] {
    if (auto* PC = GetOwningPlayer<ADrPlayerController>()) PC->ReturnToTitle();
  });
}
