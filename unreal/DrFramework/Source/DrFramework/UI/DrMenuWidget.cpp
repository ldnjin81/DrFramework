#include "UI/DrMenuWidget.h"
#include "UI/DrUISettings.h"
#include "Game/DrPlayerController.h"
UDrMenuWidget::UDrMenuWidget() { bIsBackHandler=true; }
void UDrMenuWidget::NativeOnActivated() { InputMapping=GetDefault<UDrUISettings>()->UIInputContext.LoadSynchronous(); InputMappingPriority=200; Super::NativeOnActivated(); }
TOptional<FUIInputConfig> UDrMenuWidget::GetDesiredInputConfig() const { return FUIInputConfig(ECommonInputMode::Menu,EMouseCaptureMode::NoCapture); }
UWidget* UDrMenuWidget::NativeGetDesiredFocusTarget() const { return FirstFocus ? FirstFocus.Get() : Super::NativeGetDesiredFocusTarget(); }
bool UDrMenuWidget::NativeOnHandleBackAction() { if(auto* PC=GetOwningPlayer<ADrPlayerController>()) PC->BackMenu(); return true; }
FReply UDrMenuWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) {
 if(Event.GetKey()==EKeys::Escape || Event.GetKey()==EKeys::Gamepad_Special_Right) { if(!Event.IsRepeat()) NativeOnHandleBackAction(); return FReply::Handled(); }
 return Super::NativeOnPreviewKeyDown(Geometry,Event);
}
