using UnrealBuildTool;
using System.IO;

public class DrFramework : ModuleRules
{
	public DrFramework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 모듈 자신의 폴더를 인클루드 루트로 취급해 헤더를 Source/DrFramework 기준
		// 상대경로로 인클루드할 수 있게 한다(예: "Characters/DrCharacterBase.h").
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
            "AudioMixer",
            "Niagara",
            "AudioModulation",
			"AnimGraphRuntime",
            "EngineCameras",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"UMG",
            "CommonUI",
            "CommonInput",
            "CinematicCamera",
            "Json",
			"ModelViewViewModel",
			"FieldNotification",
			"DrTableRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore"
		});
		PrivateDependencyModuleNames.Add("NavigationSystem");
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
            PrivateDependencyModuleNames.Add("AssetRegistry");
		}
	}
}
