// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Chimera_UE5 : ModuleRules
{
	public Chimera_UE5(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject",
			"Engine", "InputCore", "EnhancedInput",
            "AIModule", "StateTreeModule", "GameplayStateTreeModule", "GameplayTags",
			"PCG",
            "GeometryFramework",
            "UMG"
		});

		//PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");


		PublicDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "GameplayTags", });
		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayTasks" });

		// mesh output
		PrivateDependencyModuleNames.AddRange(new string[] {
			"AssetRegistry",
			"MeshDescription",
			"StaticMeshDescription",
		});
		if (Target.bBuildEditor){
			PrivateDependencyModuleNames.AddRange(new string[] {
				"UnrealEd",
				"AssetTools",
				"EditorScriptingUtilities", "EditorSubsystem", "EditorFramework"
			});
		}

        PublicIncludePaths.AddRange(new string[] {
				Path.Combine(ModuleDirectory, "../Chimera_UE5")
		});
		PrivateIncludePaths.AddRange(new string[] {
				Path.Combine(ModuleDirectory, "../Chimera_UE5")
		});
	}
}
