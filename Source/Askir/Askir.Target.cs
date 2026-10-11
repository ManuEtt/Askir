using UnrealBuildTool;
public class AskirTarget : TargetRules
{
    public AskirTarget(TargetInfo Target) : base(Target)
    {
        Type=TargetType.Game;
        DefaultBuildSettings=BuildSettingsVersion.V5;
        IncludeOrderVersion=EngineIncludeOrderVersion.Unreal5_6;
        ExtraModuleNames.Add("Askir");
    }
}