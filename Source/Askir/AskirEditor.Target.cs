using UnrealBuildTool;
public class AskirEditorTarget : TargetRules
{
    public AskirEditorTarget(TargetInfo Target) : base(Target)
    {
        Type=TargetType.Editor;
        DefaultBuildSettings=BuildSettingsVersion.V5;
        IncludeOrderVersion=EngineIncludeOrderVersion.Unreal5_6;
        ExtraModuleNames.Add("Askir");
    }
}