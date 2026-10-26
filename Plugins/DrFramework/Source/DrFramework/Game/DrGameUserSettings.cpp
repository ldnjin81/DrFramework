#include "Game/DrGameUserSettings.h"

UDrGameUserSettings* UDrGameUserSettings::Get() {
  return Cast<UDrGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}
float UDrGameUserSettings::GetVolume(EDrAudioChannel Channel) const {
  const float Value = Channel == EDrAudioChannel::SFX ? SFXVolume :
      Channel == EDrAudioChannel::Voice ? VoiceVolume : MusicVolume;
  return FMath::IsFinite(Value) ? FMath::Clamp(Value, 0.f, 1.f) : 1.f;
}
void UDrGameUserSettings::SetVolume(EDrAudioChannel Channel, float Value) {
  Value = FMath::IsFinite(Value) ? FMath::Clamp(Value, 0.f, 1.f) : 1.f;
  switch (Channel) {
  case EDrAudioChannel::SFX: SFXVolume = Value; break;
  case EDrAudioChannel::Voice: VoiceVolume = Value; break;
  case EDrAudioChannel::Music: MusicVolume = Value; break;
  default: return;
  }
}
void UDrGameUserSettings::SetToDefaults() {
  Super::SetToDefaults();
  SFXVolume = VoiceVolume = MusicVolume = 1.f;
}
