#include "UI/DrSettingsMenuWidget.h"

#include "UI/DrGlassButton.h"
#include "Audio/DrAudioSubsystem.h"
#include "Game/DrGameUserSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

void UDrSettingsMenuWidget::BuildMenu(UVerticalBox* Contents) {
  Sliders.Reset(); AddHeading(Contents, TEXT("Settings"));
  const TCHAR* Keys[] = {TEXT("SFX"), TEXT("Voice"), TEXT("Music")};
  auto* Settings = UDrGameUserSettings::Get();
  for (int32 I = 0; I < 3; ++I) {
    auto* Text = WidgetTree->ConstructWidget<UTextBlock>();
    Text->SetText(MenuText(Keys[I])); Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 22));
    ApplyItemSpacing(Contents->AddChildToVerticalBox(Text));
    auto* Slider = WidgetTree->ConstructWidget<USlider>();
    Slider->SetStepSize(.05f);
    Slider->SetValue(Settings ? Settings->GetVolume(static_cast<EDrAudioChannel>(I)) : 1.f);
    auto* LayoutSlot = Contents->AddChildToVerticalBox(Slider);
    LayoutSlot->SetPadding(FMargin(48, 6, 48, 18));
    Sliders.Add(Slider);
  }
  Sliders[0]->OnValueChanged.AddDynamic(this, &ThisClass::SFXChanged);
  Sliders[1]->OnValueChanged.AddDynamic(this, &ThisClass::VoiceChanged);
  Sliders[2]->OnValueChanged.AddDynamic(this, &ThisClass::MusicChanged);
  AddButton(Contents, TEXT("Back"))->OnClicked().AddWeakLambda(this, [this] { NativeOnHandleBackAction(); });
  FirstFocus = Sliders[0];
}
void UDrSettingsMenuWidget::SetVolume(EDrAudioChannel Channel, float Value) {
  if (auto* Settings = UDrGameUserSettings::Get()) {
    Settings->SetVolume(Channel, Value); Settings->SaveSettings();
    Value = Settings->GetVolume(Channel);
  }
  if (auto* Audio = GetWorld()->GetSubsystem<UDrAudioSubsystem>()) Audio->SetChannelVolume(Channel, Value);
}
USlider* UDrSettingsMenuWidget::GetSlider(EDrAudioChannel Channel) const {
  const int32 Index = static_cast<int32>(Channel);
  return Sliders.IsValidIndex(Index) ? Sliders[Index].Get() : nullptr;
}
void UDrSettingsMenuWidget::SFXChanged(float Value) { SetVolume(EDrAudioChannel::SFX, Value); }
void UDrSettingsMenuWidget::VoiceChanged(float Value) { SetVolume(EDrAudioChannel::Voice, Value); }
void UDrSettingsMenuWidget::MusicChanged(float Value) { SetVolume(EDrAudioChannel::Music, Value); }
