// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "EngineMinimal.h"
#include "CoreMinimal.h"
#include "VideoCreator.generated.h"

/**
 * 
 */
UCLASS()
class BLANKPROJECT_API UVideoCreator:public UObject
{
	GENERATED_BODY()



public:
	//UFUNCTION(BlueprintCallable, Category = "Video Creator")
	//	static void VideoMaker(FString& assetname, FString& assetid);
	UFUNCTION(BlueprintPure, Category = "VideoMaker" )
		static bool VideoMaker(FString& assetname);
};
