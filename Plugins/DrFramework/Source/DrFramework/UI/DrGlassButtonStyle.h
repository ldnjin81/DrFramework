#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "DrGlassButtonStyle.generated.h"

/** 기본 버튼 브러시를 모두 끈다. 모양은 UDrGlassButton의 유리 머티리얼이 그린다. */
UCLASS()
class DRFRAMEWORK_API UDrGlassButtonStyle : public UCommonButtonStyle {
  GENERATED_BODY()
public:
  UDrGlassButtonStyle();
};
