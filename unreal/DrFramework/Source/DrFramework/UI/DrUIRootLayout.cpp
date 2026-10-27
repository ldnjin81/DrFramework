#include "UI/DrUIRootLayout.h"

#include "Blueprint/WidgetTree.h"
#include "CommonActivatableWidget.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

TSharedRef<SWidget> UDrUIRootLayout::RebuildWidget() {
  auto* Root = WidgetTree->ConstructWidget<UOverlay>();
  WidgetTree->RootWidget = Root;
  GameStack = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>();
  MenuStack = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>();
  for (auto* Stack : {GameStack.Get(), MenuStack.Get()}) {
    Stack->SetTransitionDuration(0.f);
    auto* LayoutSlot = Root->AddChildToOverlay(Stack);
    LayoutSlot->SetHorizontalAlignment(HAlign_Fill); LayoutSlot->SetVerticalAlignment(VAlign_Fill);
  }
  return Super::RebuildWidget();
}
UCommonActivatableWidget* UDrUIRootLayout::PushGame(TSubclassOf<UCommonActivatableWidget> WidgetClass) {
  return GameStack ? GameStack->AddWidget(WidgetClass) : nullptr;
}
UCommonActivatableWidget* UDrUIRootLayout::PushMenu(TSubclassOf<UCommonActivatableWidget> WidgetClass) {
  return MenuStack ? MenuStack->AddWidget(WidgetClass) : nullptr;
}
void UDrUIRootLayout::PopMenu() {
  if (MenuStack) if (auto* Widget = MenuStack->GetActiveWidget()) Widget->DeactivateWidget();
}
void UDrUIRootLayout::ClearMenus() { if (MenuStack) MenuStack->ClearWidgets(); }
