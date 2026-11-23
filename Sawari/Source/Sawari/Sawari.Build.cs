using UnrealBuildTool;
public class Sawari : ModuleRules
{
    public Sawari(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "ChaosVehicles"
        });
    }
}
