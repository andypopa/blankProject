// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Object.h"
//#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine.h"
#include "UObjectBase.h"
#include "CoreMinimal.h"
#include "EngineMinimal.h"


//Materials

#include "Developer/AssetTools/Private/AssetRenameManager.h"

#include "MaterialCreator.generated.h"

/**
*
*/
UCLASS()
class BLANKPROJECT_API UMaterialCreator : public UObject
{
	GENERATED_BODY()
public:
	enum ETextureType { BaseColor, Specular, Roughness, Normal, AmbientOcclusion, None };

	UFUNCTION(BlueprintPure, Category = "Material Creator")
		static bool CreateMaterial(UMaterial*& Material, UPackage*& MaterialAssetPackage, FString Id);
	UFUNCTION(BlueprintCallable, Category = "Save")
		static void SaveAsset(UObject* MaterialAsset, UPackage* MaterialAssetPackage, FString Id);
	static UMaterialCreator::ETextureType GetTextureType(FAssetData TextureAssetData);
	static TTuple<UMaterial*, UPackage*> CreateMaterialInternal(FString Id);
	static TArray<FAssetData> GetAssetDataList(bool bEngine, FString Id);
	static FAssetData GetDiffuseTextureAssetData(TArray<FAssetData> AssetDataList);
	static FExpressionInput& GetExpressionInput(ETextureType TextureType, UMaterial* Material);
	static void AssignTextureToMaterial(FAssetData TextureAssetData, UMaterial* Material, FString Id);
	static void RenameAsset(FAssetData AssetData, FString Id);
};

