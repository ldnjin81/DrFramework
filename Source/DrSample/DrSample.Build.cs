using UnrealBuildTool;
public class DrSample : ModuleRules
{
 public DrSample(ReadOnlyTargetRules Target) : base(Target)
 {
  PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
  PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "DrFramework" });
 }
}
