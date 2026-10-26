#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DrPerfSubsystem.generated.h"
UCLASS()
class DRFRAMEWORK_API UDrPerfSubsystem : public UGameInstanceSubsystem { GENERATED_BODY()
protected:
 struct FSample { double Frame=0; float Game=0,GPU=0; };
 TArray<FSample> Samples[4];
 static TSharedPtr<FJsonObject> Summarize(TArray<double> Values);
};
