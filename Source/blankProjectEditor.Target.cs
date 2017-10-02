using UnrealBuildTool;
using System.Collections.Generic;

public class blankProjectEditorTarget : TargetRules
{
    public blankProjectEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;

        ExtraModuleNames.AddRange(new string[] { "blankProject" });
    }
}