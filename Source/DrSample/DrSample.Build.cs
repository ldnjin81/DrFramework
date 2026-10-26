using UnrealBuildTool;
public class DrSample : ModuleRules
{
 public DrSample(ReadOnlyTargetRules Target) : base(Target)
 {
  PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
  PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "DrFramework" });
  PublicDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "GameplayTags", "EnhancedInput", "InputCore", "AnimGraphRuntime" });
  if (Target.bBuildEditor)
   PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "AnimGraph", "BlueprintGraph", "KismetCompiler", "AssetRegistry", "NavigationSystem" });
 }
}
