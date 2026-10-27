#include "Game/DrGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
void UDrGameInstance::OnStart() { Super::OnStart(); StartBootFlow(); }
void UDrGameInstance::Shutdown() { bBootRunning=false; if(GetWorld()) GetWorld()->GetTimerManager().ClearTimer(BootTimeout); Super::Shutdown(); }
UDrGameInstance* UDrGameInstance::Get(const UObject* Context) { return Context ? Cast<UDrGameInstance>(UGameplayStatics::GetGameInstance(Context)) : nullptr; }
void UDrGameInstance::StartBootFlow() { if(bBootRunning) return; BootSteps=GetBootSteps(); BootIndex=0; bBootRunning=true; RunNextStep(); }
void UDrGameInstance::RunNextStep() {
 if(!BootSteps.IsValidIndex(BootIndex)) { bBootRunning=false; OnBootFinished(true); return; }
 if(GetWorld()) GetWorld()->GetTimerManager().SetTimer(BootTimeout, FTimerDelegate::CreateWeakLambda(this,[this] { CompleteBootStep(false); }),StepTimeoutSeconds,false);
 ExecuteBootStep(BootSteps[BootIndex]);
}
void UDrGameInstance::CompleteBootStep(bool bSucceeded) {
 if(!bBootRunning) return;
 if(GetWorld()) GetWorld()->GetTimerManager().ClearTimer(BootTimeout);
 if(!bSucceeded) { bBootRunning=false; OnBootFinished(false); return; }
 ++BootIndex; RunNextStep();
}
