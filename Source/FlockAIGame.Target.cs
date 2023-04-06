// Flock AI - Steering Behaviors for Unreal - juaxix

using UnrealBuildTool;
using System.Collections.Generic;

public class FlockAIGameTarget : TargetRules
{
	public FlockAIGameTarget(TargetInfo Target): base(Target)
	{
		Type = TargetType.Game;
		ShadowVariableWarningLevel = WarningLevel.Warning;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange( new string[] { "FlockAIGame" } );
	}
}
