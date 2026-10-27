#pragma once
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "DrMenuWidget.generated.h"
UCLASS(Abstract)
class DRFRAMEWORK_API UDrMenuWidget : public UCommonActivatableWidget { GENERATED_BODY()
public:
 UDrMenuWidget();
 virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Dr|Focus") TObjectPtr<UWidget> FirstFocus;
protected:
 virtual void NativeOnActivated() override;
 virtual UWidget* NativeGetDesiredFocusTarget() const override;
 virtual bool NativeOnHandleBackAction() override;
 virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
};
