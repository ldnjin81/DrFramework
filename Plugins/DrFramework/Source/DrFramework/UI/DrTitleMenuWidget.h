#pragma once

#include "CoreMinimal.h"
#include "UI/DrMenuWidget.h"
#include "DrTitleMenuWidget.generated.h"

/** 타이틀 메뉴. 시작·설정·종료를 고르며 뒤로가기는 무시한다. */
UCLASS()
class DRFRAMEWORK_API UDrTitleMenuWidget : public UDrMenuWidget {
  GENERATED_BODY()
public:
  void StartGame();
protected:
  virtual void BuildMenu(UVerticalBox* Contents) override;
  virtual bool NativeOnHandleBackAction() override { return true; }
};
