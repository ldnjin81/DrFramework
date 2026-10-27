#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "Dom/JsonObject.h"
#include "DrSmokeSubsystem.generated.h"
enum class EDrSmokeResult : uint8 { Pending, Passed, Failed };
UCLASS()
class DRFRAMEWORK_API UDrSmokeSubsystem : public UGameInstanceSubsystem { GENERATED_BODY()
public:
 /** 단계를 등록 순서대로 실행한다. 이름은 결과 JSON의 키로 대소문자 그대로 쓰인다. */
 void RegisterStep(const FString& Name,TFunction<EDrSmokeResult(float)> Check,float TimeoutSeconds=30);
 /** 결과 JSON(passed + 등록 순서의 steps)을 만든다. */
 static TSharedRef<FJsonObject> MakeResultJson(bool bPassed,const TArray<TPair<FString,bool>>& StepResults);
 void StartSmoke();
 virtual void Deinitialize() override;
protected:
 virtual void OnSmokeFinished(bool bPassed);
private:
 struct FStep { FString Name; TFunction<EDrSmokeResult(float)> Check; float Timeout; };
 TArray<FStep> RegisteredSteps;
 // FName은 대소문자를 보존하지 않아(먼저 등록된 표기를 따름) 결과 키가 바뀔 수 있으므로 FString으로 둔다.
 TArray<TPair<FString,bool>> Results;
 int32 CurrentStep=0;
 float Elapsed=0;
 FTSTicker::FDelegateHandle Ticker;
 bool Tick(float DeltaTime);
};
