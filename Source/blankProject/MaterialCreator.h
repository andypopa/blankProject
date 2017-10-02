// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Object.h"
//#include "Kismet/BlueprintFunctionLibrary.h"
#include "Materials/Material.h"
#include "Engine.h"
#include "UObjectBase.h"
#include "CoreMinimal.h"
#include "EngineMinimal.h"
#include "Misc/AutomationTest.h"
#include "MaterialGraph/MaterialGraph.h"
#include "Engine/Texture.h"
#include "AssetData.h"
#include "Toolkits/AssetEditorManager.h"
//Automation
#include "Tests/AutomationTestSettings.h"
#include "Tests/AutomationEditorCommon.h"
#include "Tests/AutomationEditorPromotionCommon.h"
//Assets
//Materials
#include "Editor/MaterialEditor/Private/MaterialEditor.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Tests/AutomationEditorPromotionCommon.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyPortFlags.h"
#include "Input/Events.h"
#include "Widgets/SWidget.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/InputBindingManager.h"
#include "Materials/Material.h"
#include "Editor/UnrealEdEngine.h"
#include "Editor/UnrealEd/Classes/Factories/MaterialFactoryNew.h"
#include "UnrealEdGlobals.h"
#include "Tests/AutomationCommon.h"
#include "AssetRegistryModule.h"
#include "Engine/Texture.h"
#include "LevelEditor.h"
#include "ScopedTransaction.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionConstant.h"
#include "ContentBrowserModule.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "UObject/ObjectMacros.h"
#include "Misc/Guid.h"
#include "RenderCommandFence.h"
#include "Templates/ScopedPointer.h"
#include "Materials/MaterialInterface.h"
#include "MaterialShared.h"
#include "MaterialExpressionIO.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Materials/MaterialFunction.h"
#include "UniquePtr.h"

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

	UFUNCTION(BlueprintCallable, Category = "Material Creator")
		static bool CreateMaterial(UMaterial*& Material, UPackage*& MaterialAssetPackage, FString& MaterialName);
	UFUNCTION(BlueprintCallable, Category = "Save")
		static void SaveAsset(UObject* MaterialAsset, UPackage* MaterialAssetPackage);
	static UMaterialCreator::ETextureType GetTextureType(FAssetData TextureAssetData);
	static TTuple<UMaterial*, UPackage*> CreateMaterialInternal();
	static TArray<FAssetData> GetAssetDataList(bool bEngine);
	static FAssetData GetDiffuseTextureAssetData(TArray<FAssetData> AssetDataList);
	static FExpressionInput& GetExpressionInput(ETextureType TextureType, UMaterial* Material);
	static void AssignTextureToMaterial(FAssetData TextureAssetData, UMaterial* Material);
	static void RenameAsset(FAssetData AssetData);
};

