// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
#include "Object.h"
#include "PakAsset.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UPakAsset : public UObject
{
	GENERATED_BODY()
public:
	//PakAsset();
	//~PakAsset();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PakAsset")
	FString Id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PakAsset")
	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PakAsset")
	FString Pak;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PakAsset")
	FString Sig;
};
