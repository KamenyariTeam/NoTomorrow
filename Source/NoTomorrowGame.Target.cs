// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NoTomorrowGameTarget : TargetRules
{
	public NoTomorrowGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("NoTomorrowGame");
	}
}
