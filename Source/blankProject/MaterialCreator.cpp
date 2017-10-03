// Fill out your copyright notice in the Description page of Project Settings.
#include "MaterialCreator.h"
#include "blankProject.h"


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

#include "Developer/AssetTools/Private/AssetRenameManager.h"
#include "MobilikonAssetRenameManager.h"

UMaterialCreator::ETextureType UMaterialCreator::GetTextureType(FAssetData TextureAssetData)
{
	auto Name = TextureAssetData.GetAsset()->GetName();
	if (Name.StartsWith("T_") && Name.EndsWith("_D")) return ETextureType::BaseColor;
	if (Name.StartsWith("T_") && Name.EndsWith("_N")) return ETextureType::Normal;
	if (Name.StartsWith("T_") && Name.EndsWith("_R")) return ETextureType::Roughness;
	if (Name.StartsWith("T_") && Name.EndsWith("_S")) return ETextureType::Specular;
	if (Name.StartsWith("T_") && Name.EndsWith("_O")) return ETextureType::AmbientOcclusion;
	return ETextureType::None;
}

FExpressionInput& UMaterialCreator::GetExpressionInput(ETextureType TextureType, UMaterial* Material)
{
	if (TextureType == ETextureType::BaseColor) return (FExpressionInput&)Material->BaseColor;
	if (TextureType == ETextureType::Normal) return (FExpressionInput&)Material->Normal;
	if (TextureType == ETextureType::Roughness) return (FExpressionInput&)Material->Roughness;
	if (TextureType == ETextureType::Specular) return (FExpressionInput&)Material->Specular;
	if (TextureType == ETextureType::AmbientOcclusion) return (FExpressionInput&)Material->AmbientOcclusion;
	return *(new FExpressionInput());
}

void UMaterialCreator::RenameAsset(FAssetData AssetData, FString Id)
{
	auto Asset = AssetData.GetAsset();
	auto Path = AssetData.ObjectPath.ToString();
	//fileManager.MakeDirectory(TEXT("/Engine/" + Id));
	auto Dest = Path.Replace(TEXT("/Game"), TEXT("/Engine"));

	//renaming logic
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<FAssetRenameData> AssetsAndNames;
	const FString PackagePath = FPackageName::GetLongPackagePath(Asset->GetOutermost()->GetName());
	new(AssetsAndNames)FAssetRenameData(Asset, FString("/Engine/" + Id), Asset->GetName());
	TSharedRef<MobilikonAssetRenameManager> AssetRenameManager = MakeShareable(new MobilikonAssetRenameManager);
	AssetRenameManager->RenameAssets(AssetsAndNames);
	//UMaterialCreator::RenameAssets(AssetsAndNames);
	FAssetRegistryModule::AssetRenamed(Asset, Path);
	Asset->MarkPackageDirty();
	Asset->GetOuter()->MarkPackageDirty();
	UE_LOG(LogTemp, Warning, TEXT("New Name: %s"), *Asset->GetFullName());
}
//
//void UMaterialCreator::RenameAssets(TArray<FAssetRenameData>& AssetsAndNames)
//{
//	TSharedRef<FAssetRenameManager> AssetRenameManager = MakeShareable(new FAssetRenameManager);
//	// Prep a list of assets to rename with an extra boolean to determine if they should leave a redirector or not
//	TArray<FAssetRenameDataWithReferencers> AssetsToRename;
//	AssetsToRename.Reset(AssetsAndNames.Num());
//	for (const FAssetRenameData& AssetRenameData : AssetsAndNames)
//	{
//		AssetsToRename.Emplace(FAssetRenameDataWithReferencers(AssetRenameData));
//	}
//
//	// Warn the user if they are about to rename an asset that is referenced by a CDO
//	auto CDOAssets = FindCDOReferencedAssets(AssetsToRename);
//
//	// Warn the user if there were any references
//	if (CDOAssets.Num())
//	{
//		FString AssetNames;
//		for (auto AssetIt = CDOAssets.CreateConstIterator(); AssetIt; ++AssetIt)
//		{
//			UObject* Asset = (*AssetIt).Get();
//			if (Asset)
//			{
//				AssetNames += FString("\n") + Asset->GetName();
//			}
//		}
//
//		const FText MessageText = FText::Format(LOCTEXT("RenameCDOReferences", "The following assets are referenced by one or more Class Default Objects: \n{0}\n\nContinuing with the rename may require code changes to fix these references. Do you wish to continue?"), FText::FromString(AssetNames));
//		if (FMessageDialog::Open(EAppMsgType::YesNo, MessageText) == EAppReturnType::No)
//		{
//			return;
//		}
//	}
//
//	// Fill out the referencers for the assets we are renaming
//	PopulateAssetReferencers(AssetsToRename);
//
//	// Update the source control state for the packages containing the assets we are renaming if source control is enabled. If source control is enabled and this fails we can not continue.
//	if (UpdatePackageStatus(AssetsToRename))
//	{
//		// Detect whether the assets are being referenced by a collection. Assets within a collection must leave a redirector to avoid the collection losing its references.
//		DetectReferencingCollections(AssetsToRename);
//
//		// Load all referencing packages and mark any assets that must have redirectors.
//		TArray<UPackage*> ReferencingPackagesToSave;
//		LoadReferencingPackages(AssetsToRename, ReferencingPackagesToSave);
//
//		// Prompt to check out source package and all referencing packages, leave redirectors for assets referenced by packages that are not checked out and remove those packages from the save list.
//		const bool bUserAcceptedCheckout = CheckOutPackages(AssetsToRename, ReferencingPackagesToSave);
//
//		if (bUserAcceptedCheckout)
//		{
//			// If any referencing packages are left read-only, the checkout failed or SCC was not enabled. Trim them from the save list and leave redirectors.
//			DetectReadOnlyPackages(AssetsToRename, ReferencingPackagesToSave);
//
//			// Perform the rename, leaving redirectors only for assets which need them
//			PerformAssetRename(AssetsToRename);
//
//			// Save all packages that were referencing any of the assets that were moved without redirectors
//			SaveReferencingPackages(ReferencingPackagesToSave);
//
//			// Issue post rename event
//			AssetPostRenameEvent.Broadcast(AssetsAndNames);
//		}
//	}
//
//	// Finally, report any failures that happened during the rename
//	ReportFailures(AssetsToRename);
//}

void UMaterialCreator::AssignTextureToMaterial(FAssetData TextureAssetData, UMaterial* Material, FString Id)
{
	UMaterialCreator::ETextureType TextureType = UMaterialCreator::GetTextureType(TextureAssetData);

	//return if diffuse texture encountered
	//(already bound when creating the material using the factory)
	if (TextureType == UMaterialCreator::ETextureType::BaseColor) return;

	auto Path = TextureAssetData.ObjectPath.ToString();

	UMaterialExpressionTextureSample* TextureSampler = NewObject<UMaterialExpressionTextureSample>(Material);
	UTexture* Texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), NULL, *Path));

	TextureSampler->MaterialExpressionEditorX = -500;
	TextureSampler->MaterialExpressionEditorY = TextureType * 250;
	TextureSampler->Texture = Texture;
	TextureSampler->AutoSetSampleType();

	if (TextureType == UMaterialCreator::ETextureType::Normal) 
		TextureSampler->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Normal;
	else
		TextureSampler->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Color;

	Material->Expressions.Add(TextureSampler);

	FExpressionOutput& Output = TextureSampler->GetOutputs()[0];
	FExpressionInput& Input = UMaterialCreator::GetExpressionInput(TextureType, Material);

	Input.Expression = TextureSampler;
	Input.Mask = Output.Mask;
	Input.MaskR = Output.MaskR;
	Input.MaskG = Output.MaskG;
	Input.MaskB = Output.MaskB;
	Input.MaskA = Output.MaskA;

	Material->PostEditChange();
}



	bool UMaterialCreator::CreateMaterial(UMaterial*& Material, UPackage*& MaterialAssetPackage, FString Id)
	{

		IFileManager& fileManager = IFileManager::Get();

		UMaterial* CreatedMaterial = nullptr;
		UTexture* DiffuseTexture = nullptr;
		UPackage* AssetPackage = nullptr;

		TArray<FAssetData> AssetDataList;
		AssetDataList = UMaterialCreator::GetAssetDataList(false, Id);
		for (auto AssetData : AssetDataList) {
			UMaterialCreator::RenameAsset(AssetData, Id);
		}
		AssetDataList = UMaterialCreator::GetAssetDataList(true, Id);

		auto CreateMaterialInternalTuple = CreateMaterialInternal(Id);
		CreatedMaterial = CreateMaterialInternalTuple.Key;
		AssetPackage = CreateMaterialInternalTuple.Value;
		if (CreatedMaterial)
		{
			UE_LOG(LogTemp, Display, TEXT("Created new material (%s) from diffuse texture"), *CreatedMaterial->GetName());
		}
		else return false;

		Material = CreatedMaterial;
		MaterialAssetPackage = AssetPackage;

		for (auto AssetData : AssetDataList) {
			UMaterialCreator::AssignTextureToMaterial(AssetData, CreatedMaterial, Id);
		}

		if (CreatedMaterial)
		{
			CreatedMaterial->PostEditChange();
			CreatedMaterial->ForceRecompileForRendering();
			if (AssetPackage)
			{
				AssetPackage->MarkPackageDirty();
			}
			//UE_LOG(LogTemp, Display, TEXT("Can not create new texture simple for (%s) from texture (%s)"), *CreatedMaterial->GetName(), *DiffuseTexture->GetName());
		}
		else return false;

		return true;
	}

TTuple<UMaterial*, UPackage*> UMaterialCreator::CreateMaterialInternal(FString Id)
{
	auto AssetDataList = UMaterialCreator::GetAssetDataList(true, Id);
	auto DiffuseTextureAssetData = UMaterialCreator::GetDiffuseTextureAssetData(AssetDataList);
	auto DiffuseTextureAsset = DiffuseTextureAssetData.GetAsset();
	auto DiffuseTexturePath = DiffuseTextureAssetData.ObjectPath.ToString();
	auto DiffuseTexture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), NULL, *DiffuseTexturePath));

	auto MaterialAssetName = FString::Printf(TEXT("M_%s"), *Id);

	UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
	Factory->InitialTexture = DiffuseTexture;

	FString Dir = "/Engine/" + Id;
	const FString PackageName = Dir + TEXT("/") + MaterialAssetName;

	UPackage* MaterialAssetPackage = CreatePackage(NULL, *PackageName);
	EObjectFlags Flags = RF_Public | RF_Standalone;
	UObject* CreatedAsset = Factory->FactoryCreateNew(UMaterial::StaticClass(), MaterialAssetPackage, FName(*MaterialAssetName), Flags, NULL, GWarn);
	auto CreatedMaterial = Cast<UMaterial>(CreatedAsset);
	CreatedMaterial->bUsedWithStaticLighting = true;
	if (CreatedAsset)
	{
		// Notify the asset registry
		FAssetRegistryModule::AssetCreated(CreatedAsset);
		// Mark the package dirty...
		MaterialAssetPackage->MarkPackageDirty();
	}
	return MakeTuple(CreatedMaterial, MaterialAssetPackage);
}

TArray<FAssetData> UMaterialCreator::GetAssetDataList(bool bEngine, FString Id)
{
	TArray<FAssetData> AssetData;
	UObjectLibrary* ObjectLibrary = NewObject<UObjectLibrary>(UObjectLibrary::StaticClass());
	ObjectLibrary->UseWeakReferences(false);
	if (bEngine)
		ObjectLibrary->LoadAssetDataFromPath(FString(TEXT("/Engine/") + Id));
	else
		ObjectLibrary->LoadAssetDataFromPath(FString(TEXT("/Game/") + Id));

	ObjectLibrary->LoadAssetsFromAssetData();
	ObjectLibrary->GetAssetDataList(AssetData);
	return AssetData;
}

FAssetData UMaterialCreator::GetDiffuseTextureAssetData(TArray<FAssetData> AssetDataList)
{
	for (auto AssetData : AssetDataList)
	{
		auto AssetName = AssetData.GetAsset()->GetName();
		if (AssetName.StartsWith("T_") && AssetName.EndsWith("_D")) {
			return AssetData;
		}
	}
	return nullptr;
}


void UMaterialCreator::SaveAsset(UObject* MaterialAsset, UPackage* MaterialAssetPackage, FString Id)
{
	auto ECDAbsPath = FPaths::ConvertRelativePathToFull(FPaths::EngineContentDir());
	UE_LOG(LogTemp, Warning, TEXT("R2F EngineContentDir: %s"), *ECDAbsPath);
	//FString AssetPath = FString("/Engine/" + CreatedAsset->GetName());
	//UPackage *Package = CreatePackage(nullptr, *AssetPath);
	//FString FilePath = FString::Printf(TEXT("%s%s"), *AssetPath, *FPackageName::GetAssetPackageExtension());
	bool bSuccess = UPackage::SavePackage(MaterialAssetPackage, MaterialAsset, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FString(ECDAbsPath + Id + "/M_" + Id + ".uasset"));

	UE_LOG(LogTemp, Warning, TEXT("Saved Package: %s"), bSuccess ? TEXT("True") : TEXT("False"));
}