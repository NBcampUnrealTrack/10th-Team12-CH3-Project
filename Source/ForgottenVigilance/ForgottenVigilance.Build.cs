using UnrealBuildTool;

public class ForgottenVigilance : ModuleRules
{
	public ForgottenVigilance(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"UMG",
            "NavigationSystem"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
