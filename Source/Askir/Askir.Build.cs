using UnrealBuildTool;
public class Askir : ModuleRules
{
    public Askir(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core","CoreUObject","Engine","InputCore","EnhancedInput",
            "AIModule","NavigationSystem","GameplayTasks","UMG","Slate","SlateCore"
        });
    }
}