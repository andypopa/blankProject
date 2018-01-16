// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Mobilikon : ModuleRules
{
	public Mobilikon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "MeshUtilities","AssetRegistry", "Core", "CoreUObject", "Engine", "InputCore", "Http", "Json", "JsonUtilities", "UMG" });
      
        PrivateDependencyModuleNames.AddRange(new string[] { "PakFile", "RenderCore"});
        
    
        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
