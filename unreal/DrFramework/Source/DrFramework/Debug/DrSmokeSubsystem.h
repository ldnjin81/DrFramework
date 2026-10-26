#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "DrSmokeSubsystem.generated.h"
enum class EDrSmokeResult : uint8 { Pending, Passed, Failed };
UCLASS()
class DRFRAMEWORK_API UDrSmokeSubsystem : public UGameInstanceSubsystem { GENERATED_BODY()
public:
 void RegisterStep(FName Name,TFunction<EDrSmokeResult(float)> Check,float TimeoutSeconds=30);
 void StartSmoke();
 virtual void Deinitialize() override;
protected:
 virtual void OnSmokeFinished(bool bPassed);
private:
 struct FStep { FName Name; TFunction<EDrSmokeResult(float)> Check; float Timeout; };
 TArray<FStep> RegisteredSteps;
 TMap<FName,bool> Results;
 int32 CurrentStep=0;
 float Elapsed=0;
 FTSTicker::FDelegateHandle Ticker;
 bool Tick(float DeltaTime);
};
