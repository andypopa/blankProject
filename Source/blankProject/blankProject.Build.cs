// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class blankProject : ModuleRules
{
    public blankProject(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "AssetRegistry", "Core", "CoreUObject", "Engine", "InputCore", "Http", "Json", "JsonUtilities", "UMG" });

        //PrivateDependencyModuleNames.AddRange(new string[] {  });
        PrivateIncludePaths.AddRange(
            new string[] {
                "Editor/MaterialEditor/Private"
            }
        );

        PrivateIncludePathModuleNames.AddRange(
             new string[]
             {
                "AssetRegistry",
                "AssetTools",
                "Kismet",
                "EditorWidgets"
             }
         );

        PrivateDependencyModuleNames.AddRange(
             new string[] {
                "AppFramework",
                "Core",
                "CoreUObject",
                "InputCore",
                "Engine",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "ShaderCore",
                "RenderCore",
                "RHI",
                "UnrealEd",
                "MaterialUtilities",
                "PropertyEditor",
                "PakFile",
                "GraphEditor",
             }
         );

        DynamicallyLoadedModuleNames.AddRange(
             new string[] {
                "AssetTools",
                "MainFrame",
                "SceneOutliner",
                "ClassViewer",
                "ContentBrowser",
                "WorkspaceMenuStructure"
             }
         );
        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}