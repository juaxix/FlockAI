// Flock AI - Steering Behaviors for Unreal - juaxix

using UnrealBuildTool;
using System.Collections.Generic;

public class FlockAITarget : TargetRules
{
	public FlockAITarget(TargetInfo Target): base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.AddRange( new string[] { "FlockAI" } );
	}
}
