#include "UI/DrTitleMenuWidget.h"

#include "UI/DrGlassButton.h"
#include "Game/DrPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

void UDrTitleMenuWidget::BuildMenu(UVerticalBox* Contents) {
  AddHeading(Contents, TEXT("Title"));
  AddButton(Contents, TEXT("Start"))->OnClicked().AddUObject(this, &ThisClass::StartGame);
  AddButton(Contents, TEXT("Settings"))->OnClicked().AddWeakLambda(this, [this] {
    if (auto* PC = GetOwningPlayer<ADrPlayerController>()) PC->OpenSettings();
  });
  AddButton(Contents, TEXT("Quit"))->OnClicked().AddWeakLambda(this, [this] {
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
  });
}
void UDrTitleMenuWidget::StartGame() {
  if (auto* PC = GetOwningPlayer<ADrPlayerController>()) PC->StartGame();
}
