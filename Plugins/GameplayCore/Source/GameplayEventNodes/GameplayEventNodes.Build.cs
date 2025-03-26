using UnrealBuildTool;

public class GameplayEventNodes : ModuleRules
{
    public GameplayEventNodes(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "BlueprintGraph",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
	            "Core",
                "CoreUObject",
                "Engine",
                "GameplayEvents",
                "KismetCompiler",
                "PropertyEditor",
                "UnrealEd",
            }
        );
    }
}