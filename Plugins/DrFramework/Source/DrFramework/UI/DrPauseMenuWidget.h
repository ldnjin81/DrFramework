#pragma once

#include "CoreMinimal.h"
#include "UI/DrMenuWidget.h"
#include "DrPauseMenuWidget.generated.h"

/** 일시정지 메뉴. 계속·체크포인트에서 다시·설정·타이틀로를 고른다. */
UCLASS()
class DRFRAMEWORK_API UDrPauseMenuWidget : public UDrMenuWidget {
  GENERATED_BODY()
protected:
  virtual void BuildMenu(UVerticalBox* Contents) override;
};
