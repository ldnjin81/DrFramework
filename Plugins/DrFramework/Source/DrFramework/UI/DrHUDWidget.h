#pragma once
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "DrHUDWidget.generated.h"
UCLASS(Abstract)
class DRFRAMEWORK_API UDrHUDWidget : public UCommonActivatableWidget { GENERATED_BODY()
public:
 virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
 virtual void OnOwnerPawnChanged(APawn* OldPawn,APawn* NewPawn) {}
};
