// Fill out your copyright notice in the Description page of Project Settings.
#include "MaterialSlot.h"

UMaterialSlot* UMaterialSlot::CreateMaterialSlot(FString MeshName, FString MeshThumb, UMobilikonMaterial* Material, bool& IsValid)
{
	IsValid = false;
	UMaterialSlot *Object = NewObject<UMaterialSlot>();

	if (!Object) {
		UE_LOG(LogTemp, Error, TEXT("[UMaterialSlot::CreateMaterialSlot] Could not be created"));
		return NULL;
	}
	if (!Object->IsValidLowLevel()) {
		UE_LOG(LogTemp, Error, TEXT("[UMaterialSlot::CreateMaterialSlot] Created object is not valid"));
		return NULL;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UMaterialSlot::CreateMaterialSlot] An instance is created successfully"));

	Object->MeshName = FString(MeshName);
	Object->MeshThumb = FString(MeshThumb);
	Object->Material = Material;

	Object->UE4MeshName = MeshName.Replace(TEXT("."), TEXT("_")).Replace(TEXT(" "), TEXT("_"));

	IsValid = true;
	return Object;
}