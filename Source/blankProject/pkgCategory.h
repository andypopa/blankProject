// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "PakAsset.h"
#include "pkgCategory.generated.h"
/**
*
*/
UCLASS(Blueprintable, BlueprintType)
class UpkgCategory : public UPakAsset
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "pkgCategory", Meta = (DisplayName = "Get pkgCategory"))
		static UpkgCategory* CreatePkgCategory(FString _Id, FString _Name,FString status, FString parentId, FString Thumb,bool& IsValid);
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "pkgCategory")
	//	FString Id;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "pkgCategory")
	//	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "pkgCategory")
		FString status;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "pkgCategory")
		FString parentId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "pkgCategory")
		FString Thumb;
	
};