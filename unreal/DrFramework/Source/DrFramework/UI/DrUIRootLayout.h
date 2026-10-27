#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DrUIRootLayout.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;

/** 컨트롤러 수명의 게임·모달 메뉴 스택이다. */
UCLASS()
class DRFRAMEWORK_API UDrUIRootLayout : public UUserWidget {
  GENERATED_BODY()
public:
  UCommonActivatableWidget* PushGame(TSubclassOf<UCommonActivatableWidget> WidgetClass);
  UCommonActivatableWidget* PushMenu(TSubclassOf<UCommonActivatableWidget> WidgetClass);
  void PopMenu();
  void ClearMenus();
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
private:
  UPROPERTY(Transient) TObjectPtr<UCommonActivatableWidgetStack> GameStack;
  UPROPERTY(Transient) TObjectPtr<UCommonActivatableWidgetStack> MenuStack;
};
