#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Content/DrContentBase.h"
#include "DrContentSettings.generated.h"
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="DrFramework 콘텐츠"))
class DRFRAMEWORK_API UDrContentSettings : public UDeveloperSettings { GENERATED_BODY()
public:
 UPROPERTY(Config, EditAnywhere, Category="콘텐츠") TArray<TSoftClassPtr<UDrContentBase>> ContentClasses;
};
