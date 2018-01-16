// Fill out your copyright notice in the Description page of Project Settings.
#include "VideoCreator.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "EngineMinimal.h"
#include "Engine.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "StaticMeshResources.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "AsyncTaskDownloadPak.h"





	

 bool UVideoCreator::VideoMaker(FString& assetname)
{
	FString fileName;
		if (FParse::Value(FCommandLine::Get(), TEXT("assetname="), fileName)) {
		assetname = fileName;
		return true;
	}
		else
	{
		UE_LOG(LogTemp, Error, TEXT("No id token specified. Will not create video."));
		return false;
	}

	
}