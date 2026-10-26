#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "Audio/DrAudioData.h"
#include "DrGameUserSettings.generated.h"

/** 월드 전환과 다음 실행에도 유지되는 음량 설정이다. */
UCLASS(Config=GameUserSettings)
class DRFRAMEWORK_API UDrGameUserSettings : public UGameUserSettings {
  GENERATED_BODY()
public:
  static UDrGameUserSettings* Get();
  float GetVolume(EDrAudioChannel Channel) const;
  void SetVolume(EDrAudioChannel Channel, float Value);
  virtual void SetToDefaults() override;
private:
  UPROPERTY(Config) float SFXVolume = 1.f;
  UPROPERTY(Config) float VoiceVolume = 1.f;
  UPROPERTY(Config) float MusicVolume = 1.f;
};
