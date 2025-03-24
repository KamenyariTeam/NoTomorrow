// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NoTomorrowGame : ModuleRules
{
	public NoTomorrowGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(["NoTomorrowGame"]);

		PublicDependencyModuleNames.AddRange([
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "ModularGameplay", "ModularGameplayActors",
			"UMG", "DeveloperSettings"
		]);

		PrivateDependencyModuleNames.AddRange([]);
	}
}
