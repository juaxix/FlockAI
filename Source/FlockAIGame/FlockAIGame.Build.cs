// Flock AI - Steering Behaviors for Unreal - juaxix

using UnrealBuildTool;

public class FlockAIGame : ModuleRules
{
	public FlockAIGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		ShadowVariableWarningLevel = WarningLevel.Error;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		PublicDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine", "InputCore", "FlockAI"});
	}
}