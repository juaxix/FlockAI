// Flock AI - Steering Behaviors for Unreal - juaxix

using UnrealBuildTool;
using System.Collections.Generic;

public class FlockAIEditorTarget : TargetRules
{
	public FlockAIEditorTarget(TargetInfo Target): base(Target)
	{
		Type = TargetType.Editor;
		ShadowVariableWarningLevel = WarningLevel.Warning;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange( new string[] { "FlockAI" } );
	}

}
