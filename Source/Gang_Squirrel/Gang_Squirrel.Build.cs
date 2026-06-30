// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Gang_Squirrel : ModuleRules
{
	public Gang_Squirrel(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			// Default Module
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput"
			// GameAbilitiesSystem Module
			,"GameplayAbilities","GameplayTags","GameplayTasks"
			// UMG Module
			,"UMG","Slate","SlateCore"
			// AI Module
			,"AIModule","NavigationSystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
