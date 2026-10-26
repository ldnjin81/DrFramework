#include "Debug/DrSmokeSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonSerializer.h"
void UDrSmokeSubsystem::RegisterStep(FName Name,TFunction<EDrSmokeResult(float)> Check,float TimeoutSeconds) { RegisteredSteps.Add({Name,MoveTemp(Check),FMath::Max(.01f,TimeoutSeconds)}); }
void UDrSmokeSubsystem::StartSmoke() { if(Ticker.IsValid()) return; CurrentStep=0; Elapsed=0; Results.Empty(); Ticker=FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this,&ThisClass::Tick)); }
bool UDrSmokeSubsystem::Tick(float DeltaTime) {
 if(!RegisteredSteps.IsValidIndex(CurrentStep)) { OnSmokeFinished(true); Ticker.Reset(); return false; }
 Elapsed+=DeltaTime; auto& Step=RegisteredSteps[CurrentStep];
 auto Result=Step.Check(Elapsed); if(Elapsed>Step.Timeout) Result=EDrSmokeResult::Failed;
 if(Result==EDrSmokeResult::Pending) return true;
 Results.Add(Step.Name,Result==EDrSmokeResult::Passed);
 if(Result==EDrSmokeResult::Failed) { OnSmokeFinished(false); Ticker.Reset(); return false; }
 ++CurrentStep; Elapsed=0; return true;
}
void UDrSmokeSubsystem::Deinitialize() { FTSTicker::GetCoreTicker().RemoveTicker(Ticker); RegisteredSteps.Empty(); Super::Deinitialize(); }
void UDrSmokeSubsystem::OnSmokeFinished(bool bPassed) {
 auto Root=MakeShared<FJsonObject>(); Root->SetBoolField(TEXT("passed"),bPassed);
 auto Steps=MakeShared<FJsonObject>(); for(const auto& Pair:Results) Steps->SetBoolField(Pair.Key.ToString(),Pair.Value); Root->SetObjectField(TEXT("steps"),Steps);
 FString Json; FJsonSerializer::Serialize(Root,TJsonWriterFactory<>::Create(&Json));
 const FString Dir=FPaths::ProjectSavedDir()/TEXT("Smoke"); IFileManager::Get().MakeDirectory(*Dir,true);
 const bool Saved=FFileHelper::SaveStringToFile(Json,*(Dir/TEXT("result.json")));
 FPlatformMisc::RequestExitWithStatus(false,bPassed && Saved ? 0 : 1);
}
