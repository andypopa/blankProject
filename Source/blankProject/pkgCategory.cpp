// Fill out your copyright notice in the Description page of Project Settings.

#include "pkgCategory.h"
#include "MaterialSlot.h"
#include "Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"

UpkgCategory* UpkgCategory::CreatePkgCategory(FString _Id, FString _Name, FString status, FString parentId, FString Thumb, bool& IsValid)
{
	IsValid = false;
	UpkgCategory *Object = NewObject<UpkgCategory>();

	if (!Object) {
		UE_LOG(LogTemp, Error, TEXT("[UpkgCategory::Create pkgCategory] Could not be created"));
		return NULL;
	}
	if (!Object->IsValidLowLevel()) {
		UE_LOG(LogTemp, Error, TEXT("[UpkgCategory::Create pkgCategory] Created object is not valid"));
		return NULL;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UpkgCategory::Create pkgCategory] An instance is created successfully"));
	Object->Id = FString(_Id);
	Object->Name = FString(_Name);
	Object->status = FString(status);
	Object->parentId = FString(parentId);
	Object->Thumb = FString(Thumb);
	IsValid = true;
	return Object;
}
