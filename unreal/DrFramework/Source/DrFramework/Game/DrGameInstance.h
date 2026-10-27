#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "DrGameInstance.generated.h"
UCLASS()
class DRFRAMEWORK_API UDrGameInstance : public UGameInstance { GENERATED_BODY()
public:
 virtual void OnStart() override;
 virtual void Shutdown() override;
 void StartBootFlow();
 static UDrGameInstance* Get(const UObject* Context);
protected:
 virtual TArray<FName> GetBootSteps() const { return {}; }
 virtual void ExecuteBootStep(FName Step) { CompleteBootStep(true); }
 virtual void OnBootFinished(bool bSucceeded) {}
 void CompleteBootStep(bool bSucceeded);
 UPROPERTY(EditDefaultsOnly, Category="Dr|Boot") float StepTimeoutSeconds=30;
private:
 void RunNextStep();
 TArray<FName> BootSteps;
 int32 BootIndex=0;
 bool bBootRunning=false;
 FTimerHandle BootTimeout;
};
