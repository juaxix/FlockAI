// Flock AI - Steering Behaviors for Unreal - juaxix

using UnrealBuildTool;

public class FlockAI : ModuleRules
{
	public FlockAI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine", "InputCore"});
	}
}