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
 /** 실행 인자 -DrSmokeTest가 있는지. Tools/smoke_win64.ps1이 이 인자로 패키지를 실행한다. */
 static bool IsSmokeRequested();
 /** 만들어질 파생이 있으면 기반은 만들지 않는다(같은 게임 인스턴스에서 스모크가 두 번 돌지 않게). */
 virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
 virtual void Initialize(FSubsystemCollectionBase& Collection) override;
 virtual void Deinitialize() override;
protected:
 /** -DrSmokeTest로 실행됐을 때 Initialize가 부른다. 게임은 여기서 RegisterStep으로 단계를 등록하고, 끝나면 스모크가 시작된다. */
 virtual void RegisterSmokeSteps() {}
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
