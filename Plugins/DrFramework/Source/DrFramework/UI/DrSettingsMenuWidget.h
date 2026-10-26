#pragma once

#include "CoreMinimal.h"
#include "UI/DrMenuWidget.h"
#include "Audio/DrAudioData.h"
#include "DrSettingsMenuWidget.generated.h"

class USlider;

/** 설정 메뉴. 채널별 음량 슬라이더를 저장하고 즉시 반영한다. */
UCLASS()
class DRFRAMEWORK_API UDrSettingsMenuWidget : public UDrMenuWidget {
  GENERATED_BODY()
public:
  void SetVolume(EDrAudioChannel Channel, float Value);
  USlider* GetSlider(EDrAudioChannel Channel) const;
protected:
  virtual void BuildMenu(UVerticalBox* Contents) override;
private:
  UFUNCTION() void SFXChanged(float Value);
  UFUNCTION() void VoiceChanged(float Value);
  UFUNCTION() void MusicChanged(float Value);
  UPROPERTY(Transient) TArray<TObjectPtr<USlider>> Sliders;
};
