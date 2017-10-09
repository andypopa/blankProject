// Fill out your copyright notice in the Description page of Project Settings.

#include "MaterialCreatorCommandlet.h"
#include "MaterialCreator.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

UMaterialCreatorCommandlet::UMaterialCreatorCommandlet()
{
	//IsClient = false;
	IsEditor = true;
	IsServer = true;
	LogToConsole = true;
}

int32 UMaterialCreatorCommandlet::Main(const FString& CmdLine)
{
	TMap < FString, FString > Params;
    TArray < FString >  Tokens;
	TArray < FString >  Switches;
		
	
	UMaterial* Material;
	UPackage* MaterialPackage;
	FString MaterialName = FString();
	UCommandlet::ParseCommandLine(*CmdLine, Tokens, Switches, Params);

	UE_LOG(LogTemp, Display, TEXT("GEditor Address: %p"), &GEditor);
	UE_LOG(LogTemp, Display, TEXT("GUnrealEd Address: %p"), &GUnrealEd);

	FString UnrealEdEngineClassName;
	GConfig->GetString(TEXT("/Script/Engine.Engine"), TEXT("UnrealEdEngine"), UnrealEdEngineClassName, GEngineIni);
	auto EngineClass = StaticLoadClass(UUnrealEdEngine::StaticClass(), nullptr, *UnrealEdEngineClassName);
	if (EngineClass == nullptr)
	{
		UE_LOG(LogInit, Fatal, TEXT("Failed to load UnrealEd Engine class '%s'."), *UnrealEdEngineClassName);
	}
	GEngine = GEditor = GUnrealEd = NewObject<UUnrealEdEngine>(GetTransientPackage(), EngineClass);

	UE_LOG(LogTemp, Display, TEXT("GEditor Address: %p"), &GEditor);
	UE_LOG(LogTemp, Display, TEXT("GUnrealEd Address: %p"), &GUnrealEd);

	for (auto& Elem : Params)
	{
		//UE_LOG(LogTemp, Display, TEXT("%s, %s"), *Elem.Key, *Elem.Value);
		if (Elem.Key == FString("id"))
		{
			UE_LOG(LogTemp, Display, TEXT("Material ID: %s\nCreating material..."), *Elem.Value);
			UMaterialCreator::CreateMaterial(Material, MaterialPackage, Elem.Value);
			UMaterialCreator::SaveAsset(Material, MaterialPackage, Elem.Value);
			return 0;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No id token specified. Will not create material."));
			return -1;
		}
	}
	return 0;
}

