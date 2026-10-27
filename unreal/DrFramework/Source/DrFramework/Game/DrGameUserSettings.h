#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "DrGameUserSettings.generated.h"
UCLASS(Config=GameUserSettings)
class DRFRAMEWORK_API UDrGameUserSettings : public UGameUserSettings { GENERATED_BODY()
public:
 virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;
protected:
 virtual void ApplyGameSettings() {}
};
