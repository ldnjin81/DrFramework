#include "Debug/DrPerfSubsystem.h"
#include "Dom/JsonObject.h"
TSharedPtr<FJsonObject> UDrPerfSubsystem::Summarize(TArray<double> Values) {
  auto Result = MakeShared<FJsonObject>();
  Result->SetNumberField(TEXT("samples"), Values.Num());
  if (Values.IsEmpty()) {
    Result->SetField(TEXT("mean_ms"), MakeShared<FJsonValueNull>());
    Result->SetField(TEXT("p95_ms"), MakeShared<FJsonValueNull>());
    Result->SetField(TEXT("max_ms"), MakeShared<FJsonValueNull>());
    return Result;
  }
  Values.Sort();
  double Total = 0;
  for (double Value : Values) Total += Value;
  Result->SetNumberField(TEXT("mean_ms"), Total / Values.Num());
  Result->SetNumberField(TEXT("p95_ms"), Values[FMath::CeilToInt(Values.Num() * .95) - 1]);
  Result->SetNumberField(TEXT("max_ms"), Values.Last());
  return Result;
}
