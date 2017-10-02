// Fill out your copyright notice in the Description page of Project Settings.

#include "MaterialCreatorCommandlet.h"
#include "MaterialCreator.h"
UMaterialCreatorCommandlet::UMaterialCreatorCommandlet()
{
	IsClient = false;
	IsEditor = false;
	IsServer = false;
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
	for (auto& Elem : Params)
	{
		//UE_LOG(LogTemp, Display, TEXT("%s, %s"), *Elem.Key, *Elem.Value);
		if (Elem.Key == FString("id"))
		{
			UE_LOG(LogTemp, Display, TEXT("Material ID: %s\nCreating material..."), *Elem.Value);
			UMaterialCreator::CreateMaterial(Material, MaterialPackage, Elem.Value);
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

