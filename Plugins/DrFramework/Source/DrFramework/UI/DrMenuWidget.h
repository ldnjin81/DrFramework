#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "DrMenuWidget.generated.h"

class UDrGlassButton;
class UVerticalBox;
class UVerticalBoxSlot;

/** 메뉴는 입력 라우팅·첫 포커스·되돌아가기를 공통으로 처리한다. */
UCLASS(Abstract)
class DRFRAMEWORK_API UDrMenuWidget : public UCommonActivatableWidget {
  GENERATED_BODY()
public:
  UDrMenuWidget();
  virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
protected:
  virtual void NativeOnActivated() override;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual UWidget* NativeGetDesiredFocusTarget() const override;
  virtual bool NativeOnHandleBackAction() override;
  virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
  virtual void BuildMenu(UVerticalBox* Contents) PURE_VIRTUAL(UDrMenuWidget::BuildMenu, );
  UDrGlassButton* AddButton(UVerticalBox* Contents, const TCHAR* Key);
  void AddHeading(UVerticalBox* Contents, const TCHAR* Key);
  /** 메뉴 문자열 테이블에서 문구를 찾는다. */
  static FText MenuText(const TCHAR* Key);
  /** 메뉴 항목 사이 간격과 가운데 정렬을 맞춘다. */
  static void ApplyItemSpacing(UVerticalBoxSlot* LayoutSlot);
  UPROPERTY(Transient) TObjectPtr<UWidget> FirstFocus;
};
