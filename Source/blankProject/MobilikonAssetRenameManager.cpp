// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.


#include "MobilikonAssetRenameManager.h"
#include "Serialization/ArchiveUObject.h"
#include "UObject/Class.h"
#include "Misc/PackageName.h"
#include "Misc/MessageDialog.h"
#include "HAL/FileManager.h"
#include "Misc/FeedbackContext.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectIterator.h"


#include "Layout/WidgetPath.h"
#include "SlateOptMacros.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBorder.h"



#include "ISourceControlModule.h"
#include "FileHelpers.h"
//#include "SDiscoveringAssetsDialog.h"
#include "AssetRegistryModule.h"

#include "ICollectionManager.h"
#include "CollectionManagerModule.h"
#include "ObjectTools.h"
//#include "Interfaces/IMainFrameModule.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/RedirectCollector.h"
#define LOCTEXT_NAMESPACE "AssetRenameManager"

struct FAssetRenameDataWithReferencers : public FAssetRenameData
{
	TArray<FName> ReferencingPackageNames;
	FText FailureReason;
	bool bCreateRedirector;
	bool bRenameFailed;

	FAssetRenameDataWithReferencers(const FAssetRenameData& InRenameData)
		: FAssetRenameData(InRenameData)
		, bCreateRedirector(false)
		, bRenameFailed(false)
	{}
};

///////////////////////////
// FAssetRenameManager
///////////////////////////

void MobilikonAssetRenameManager::RenameAssets(const TArray<FAssetRenameData>& AssetsAndNames) const
{
	// If the asset registry is still loading assets, we cant check for referencers, so we must open the rename dialog
	//FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	//if (AssetRegistryModule.Get().IsLoadingAssets())
	//{
	//	// Open a dialog asking the user to wait while assets are being discovered
	//	SDiscoveringAssetsDialog::OpenDiscoveringAssetsDialog(
	//		SDiscoveringAssetsDialog::FOnAssetsDiscovered::CreateSP(this, &MobilikonAssetRenameManager::FixReferencesAndRename, AssetsAndNames)
	//	); 
	//}
	//else
	//{
		// No need to wait, attempt to fix references and rename now.
		FixReferencesAndRename(AssetsAndNames);
	//}
}

void MobilikonAssetRenameManager::FixReferencesAndRename(TArray<FAssetRenameData> AssetsAndNames) const
{
	// Prep a list of assets to rename with an extra boolean to determine if they should leave a redirector or not
	TArray<FAssetRenameDataWithReferencers> AssetsToRename;
	AssetsToRename.Reset(AssetsAndNames.Num());
	for (const FAssetRenameData& AssetRenameData : AssetsAndNames)
	{
		AssetsToRename.Emplace(FAssetRenameDataWithReferencers(AssetRenameData));
	}

	// Warn the user if they are about to rename an asset that is referenced by a CDO
	auto CDOAssets = FindCDOReferencedAssets(AssetsToRename);

	// Warn the user if there were any references
	if (CDOAssets.Num())
	{
		FString AssetNames;
		for (auto AssetIt = CDOAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			UObject* Asset = (*AssetIt).Get();
			if (Asset)
			{
				AssetNames += FString("\n") + Asset->GetName();
			}
		}

		const FText MessageText = FText::Format(LOCTEXT("RenameCDOReferences", "The following assets are referenced by one or more Class Default Objects: \n{0}\n\nContinuing with the rename may require code changes to fix these references. Do you wish to continue?"), FText::FromString(AssetNames));
		if (FMessageDialog::Open(EAppMsgType::YesNo, MessageText) == EAppReturnType::No)
		{
			return;
		}
	}

	// Fill out the referencers for the assets we are renaming
	PopulateAssetReferencers(AssetsToRename);

	// Update the source control state for the packages containing the assets we are renaming if source control is enabled. If source control is enabled and this fails we can not continue.
	if (UpdatePackageStatus(AssetsToRename))
	{
		// Detect whether the assets are being referenced by a collection. Assets within a collection must leave a redirector to avoid the collection losing its references.
		DetectReferencingCollections(AssetsToRename);

		// Load all referencing packages and mark any assets that must have redirectors.
		TArray<UPackage*> ReferencingPackagesToSave;
		LoadReferencingPackages(AssetsToRename, ReferencingPackagesToSave);

		// Prompt to check out source package and all referencing packages, leave redirectors for assets referenced by packages that are not checked out and remove those packages from the save list.
		const bool bUserAcceptedCheckout = CheckOutPackages(AssetsToRename, ReferencingPackagesToSave);

		if (bUserAcceptedCheckout)
		{
			// If any referencing packages are left read-only, the checkout failed or SCC was not enabled. Trim them from the save list and leave redirectors.
			DetectReadOnlyPackages(AssetsToRename, ReferencingPackagesToSave);

			// Perform the rename, leaving redirectors only for assets which need them
			PerformAssetRename(AssetsToRename);

			// Save all packages that were referencing any of the assets that were moved without redirectors
			SaveReferencingPackages(ReferencingPackagesToSave);

			// Issue post rename event
			AssetPostRenameEvent.Broadcast(AssetsAndNames);
		}
	}

	// Finally, report any failures that happened during the rename
	ReportFailures(AssetsToRename);
}

TArray<TWeakObjectPtr<UObject>> MobilikonAssetRenameManager::FindCDOReferencedAssets(const TArray<FAssetRenameDataWithReferencers>& AssetsToRename) const
{
	TArray<TWeakObjectPtr<UObject>> CDOAssets, LocalAssetsToRename;
	for (auto AssetIt = AssetsToRename.CreateConstIterator(); AssetIt; ++AssetIt)
	{
		LocalAssetsToRename.Push((*AssetIt).Asset);
	}

	// Run over all CDOs and check for any references to the assets
	for (TObjectIterator<UClass> ClassDefaultObjectIt; ClassDefaultObjectIt; ++ClassDefaultObjectIt)
	{
		UClass* Cls = (*ClassDefaultObjectIt);
		UObject* CDO = Cls->ClassDefaultObject;

		if (!CDO || !CDO->HasAllFlags(RF_ClassDefaultObject) || Cls->ClassGeneratedBy != NULL)
		{
			continue;
		}

		// Ignore deprecated and temporary trash classes.
		if (Cls->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists) || FKismetEditorUtilities::IsClassABlueprintSkeleton(Cls))
		{
			continue;
		}

		for (TFieldIterator<UObjectProperty> PropertyIt(Cls); PropertyIt; ++PropertyIt)
		{
			const UObject* Object = PropertyIt->GetPropertyValue(PropertyIt->ContainerPtrToValuePtr<UObject>(CDO));
			for (auto AssetIt = LocalAssetsToRename.CreateConstIterator(); AssetIt; ++AssetIt)
			{
				auto Asset = *AssetIt;
				if (Object == Asset.Get())
				{
					CDOAssets.Push(Asset);
					LocalAssetsToRename.Remove(Asset);

					if (LocalAssetsToRename.Num() == 0)
					{
						// No more assets to check
						return MoveTemp(CDOAssets);
					}
					else
					{
						break;
					}
				}
			}
		}
	}

	return MoveTemp(CDOAssets);
}

void MobilikonAssetRenameManager::PopulateAssetReferencers(TArray<FAssetRenameDataWithReferencers>& AssetsToPopulate) const
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TSet<FName> RenamingAssetPackageNames;

	// Get the names of all the packages containing the assets we are renaming so they arent added to the referencing packages list
	for (auto AssetIt = AssetsToPopulate.CreateConstIterator(); AssetIt; ++AssetIt)
	{
		UObject* Asset = (*AssetIt).Asset.Get();
		if (Asset)
		{
			RenamingAssetPackageNames.Add(FName(*Asset->GetOutermost()->GetName()));
		}
	}

	// Gather all referencing packages for all assets that are being renamed
	for (auto AssetIt = AssetsToPopulate.CreateIterator(); AssetIt; ++AssetIt)
	{
		(*AssetIt).ReferencingPackageNames.Empty();

		UObject* Asset = (*AssetIt).Asset.Get();
		if (Asset)
		{
			TArray<FName> Referencers;
			AssetRegistryModule.Get().GetReferencers(Asset->GetOutermost()->GetFName(), Referencers);

			for (auto ReferenceIt = Referencers.CreateConstIterator(); ReferenceIt; ++ReferenceIt)
			{
				const FName& ReferencingPackageName = *ReferenceIt;

				if (!RenamingAssetPackageNames.Contains(ReferencingPackageName))
				{
					(*AssetIt).ReferencingPackageNames.AddUnique(ReferencingPackageName);
				}
			}
		}
	}
}

bool MobilikonAssetRenameManager::UpdatePackageStatus(const TArray<FAssetRenameDataWithReferencers>& AssetsToRename) const
{
	if (ISourceControlModule::Get().IsEnabled())
	{
		ISourceControlProvider& SourceControlProvider = ISourceControlModule::Get().GetProvider();

		// Update the source control server availability to make sure we can do the rename operation
		SourceControlProvider.Login();
		if (!SourceControlProvider.IsAvailable())
		{
			FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("UnrealEd", "SourceControl_ServerUnresponsive", "Source Control is unresponsive. Please check your connection and try again."));
			return false;
		}

		// Gather asset package names to update SCC states in a single SCC request
		TArray<UPackage*> PackagesToUpdate;
		for (auto AssetIt = AssetsToRename.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			UObject* Asset = (*AssetIt).Asset.Get();
			if (Asset)
			{
				PackagesToUpdate.AddUnique(Asset->GetOutermost());
			}
		}

		SourceControlProvider.Execute(ISourceControlOperation::Create<FUpdateStatus>(), PackagesToUpdate);
	}

	return true;
}

void MobilikonAssetRenameManager::LoadReferencingPackages(TArray<FAssetRenameDataWithReferencers>& AssetsToRename, TArray<UPackage*>& OutReferencingPackagesToSave) const
{
	const FText ReferenceUpdateSlowTask = LOCTEXT("ReferenceUpdateSlowTask", "Updating Asset References");
	GWarn->BeginSlowTask(ReferenceUpdateSlowTask, true);

	ISourceControlProvider& SourceControlProvider = ISourceControlModule::Get().GetProvider();

	for (int32 AssetIdx = 0; AssetIdx < AssetsToRename.Num(); ++AssetIdx)
	{
		GWarn->StatusUpdate(AssetIdx, AssetsToRename.Num(), ReferenceUpdateSlowTask);

		FAssetRenameDataWithReferencers& RenameData = AssetsToRename[AssetIdx];

		UObject* Asset = RenameData.Asset.Get();
		if (Asset)
		{
			// Make sure this asset is local. Only local assets should be renamed without a redirector
			FSourceControlStatePtr SourceControlState = SourceControlProvider.GetState(Asset->GetOutermost(), EStateCacheUsage::ForceUpdate);
			const bool bLocalFile = !SourceControlState.IsValid() || SourceControlState->IsAdded() || !SourceControlState->IsSourceControlled() || SourceControlState->IsIgnored();
			if (!bLocalFile)
			{
				// If this asset is locked or not current, mark it failed to prevent it from being renamed
				if (SourceControlState->IsCheckedOutOther())
				{
					RenameData.bRenameFailed = true;
					RenameData.FailureReason = LOCTEXT("RenameFailedCheckedOutByOther", "Checked out by another user.");
				}
				else if (!SourceControlState->IsCurrent())
				{
					RenameData.bRenameFailed = true;
					RenameData.FailureReason = LOCTEXT("RenameFailedNotCurrent", "Out of date.");
				}

				// This asset is not local. It is not safe to rename it without leaving a redirector
				RenameData.bCreateRedirector = true;
				continue;
			}
		}
		else
		{
			// The asset for this rename must have been GCed or is otherwise invalid. Skip it
			continue;
		}

		TArray<UPackage*> PackagesToSaveForThisAsset;
		bool bAllPackagesLoadedForThisAsset = true;
		for (auto PackageNameIt = RenameData.ReferencingPackageNames.CreateConstIterator(); PackageNameIt; ++PackageNameIt)
		{
			const FString PackageName = (*PackageNameIt).ToString();

			// Check if the package is a map before loading it!
			if (FEditorFileUtils::IsMapPackageAsset(PackageName))
			{
				// This reference was a map package, don't load it and leave a redirector for this asset
				RenameData.bCreateRedirector = true;
				bAllPackagesLoadedForThisAsset = false;
				break;
			}

			UPackage* Package = LoadPackage(NULL, *PackageName, LOAD_None);
			if (Package)
			{
				PackagesToSaveForThisAsset.Add(Package);
			}
			else
			{
				RenameData.bCreateRedirector = true;
				bAllPackagesLoadedForThisAsset = false;
				break;
			}
		}

		if (bAllPackagesLoadedForThisAsset)
		{
			OutReferencingPackagesToSave.Append(PackagesToSaveForThisAsset);
		}
	}

	GWarn->EndSlowTask();
}

bool MobilikonAssetRenameManager::CheckOutPackages(TArray<FAssetRenameDataWithReferencers>& AssetsToRename, TArray<UPackage*>& InOutReferencingPackagesToSave) const
{
	return true;
	bool bUserAcceptedCheckout = true;

	// Build list of packages to check out: the source package and any referencing packages (in the case that we do not create a redirector)
	TArray<UPackage*> PackagesToCheckOut;
	PackagesToCheckOut.Reset(AssetsToRename.Num() + InOutReferencingPackagesToSave.Num());

	for (const FAssetRenameDataWithReferencers& AssetToRename : AssetsToRename)
	{
		if (!AssetToRename.bRenameFailed && AssetToRename.Asset.IsValid())
		{
			PackagesToCheckOut.Add(AssetToRename.Asset->GetOutermost());
		}
	}

	for (UPackage* ReferencingPackage : InOutReferencingPackagesToSave)
	{
		PackagesToCheckOut.Add(ReferencingPackage);
	}

	// Check out the packages
	if (PackagesToCheckOut.Num() > 0)
	{
		TArray<UPackage*> PackagesCheckedOutOrMadeWritable;
		TArray<UPackage*> PackagesNotNeedingCheckout;
		bUserAcceptedCheckout = FEditorFileUtils::PromptToCheckoutPackages(false, PackagesToCheckOut, &PackagesCheckedOutOrMadeWritable, &PackagesNotNeedingCheckout);
		if (bUserAcceptedCheckout)
		{
			// Make a list of any packages in the list which weren't checked out for some reason
			TArray<UPackage*> PackagesThatCouldNotBeCheckedOut = PackagesToCheckOut;

			for (auto PackageIt = PackagesCheckedOutOrMadeWritable.CreateConstIterator(); PackageIt; ++PackageIt)
			{
				PackagesThatCouldNotBeCheckedOut.Remove(*PackageIt);
			}

			for (auto PackageIt = PackagesNotNeedingCheckout.CreateConstIterator(); PackageIt; ++PackageIt)
			{
				PackagesThatCouldNotBeCheckedOut.Remove(*PackageIt);
			}

			// If there's anything which couldn't be checked out, abort the operation.
			if (PackagesThatCouldNotBeCheckedOut.Num() > 0)
			{
				bUserAcceptedCheckout = false;
			}
		}
	}

	return bUserAcceptedCheckout;
}

void MobilikonAssetRenameManager::DetectReferencingCollections(TArray<FAssetRenameDataWithReferencers>& AssetsToRename) const
{
	FCollectionManagerModule& CollectionManagerModule = FCollectionManagerModule::GetModule();

	for (auto& AssetToRename : AssetsToRename)
	{
		if (AssetToRename.Asset.IsValid())
		{
			TArray<FCollectionNameType> ReferencingCollections;
			CollectionManagerModule.Get().GetCollectionsContainingObject(*AssetToRename.Asset->GetPathName(), ReferencingCollections);

			if (ReferencingCollections.Num() > 0)
			{
				AssetToRename.bCreateRedirector = true;
			}
		}
	}
}

void MobilikonAssetRenameManager::DetectReadOnlyPackages(TArray<FAssetRenameDataWithReferencers>& AssetsToRename, TArray<UPackage*>& InOutReferencingPackagesToSave) const
{
	// For each valid package...
	for (int32 PackageIdx = InOutReferencingPackagesToSave.Num() - 1; PackageIdx >= 0; --PackageIdx)
	{
		UPackage* Package = InOutReferencingPackagesToSave[PackageIdx];

		if (Package)
		{
			// Find the package filename
			FString Filename;
			if (FPackageName::DoesPackageExist(Package->GetName(), NULL, &Filename))
			{
				// If the file is read only
				if (IFileManager::Get().IsReadOnly(*Filename))
				{
					FName PackageName = Package->GetFName();

					// Find all assets that were referenced by this package to create a redirector when named
					for (auto RenameDataIt = AssetsToRename.CreateIterator(); RenameDataIt; ++RenameDataIt)
					{
						FAssetRenameDataWithReferencers& RenameData = *RenameDataIt;
						if (RenameData.ReferencingPackageNames.Contains(PackageName))
						{
							RenameData.bCreateRedirector = true;
						}
					}

					// Remove the package from the save list
					InOutReferencingPackagesToSave.RemoveAt(PackageIdx);
				}
			}
		}
	}
}

void MobilikonAssetRenameManager::RenameReferencingStringAssetReferences(const TArray<UPackage *> PackagesToCheck, const TMap<FSoftObjectPath, FSoftObjectPath>& AssetRedirectorMap)
{
	struct FStringAssetReferenceRenameSerializer : public FArchiveUObject
	{
		FStringAssetReferenceRenameSerializer()
		{
			// Mark it as saving to correctly process all references
			ArIsSaving = true;
		}
	};

	// Add redirects as needed
	for (const TPair<FSoftObjectPath, FSoftObjectPath>& Pair : AssetRedirectorMap)
	{
	//GRedirectCollector.AddAssetPathRedirection(Pair.Key, Pair.Value);
	GRedirectCollector.AddAssetPathRedirection(Pair.Key.GetAssetPathName(), Pair.Value.GetAssetPathName());
	}

	FStringAssetReferenceRenameSerializer RenameSerializer;

	for (auto* Package : PackagesToCheck)
	{
		TArray<UObject*> ObjectsInPackage;
		GetObjectsWithOuter(Package, ObjectsInPackage);

		for (auto* Object : ObjectsInPackage)
		{
			Object->Serialize(RenameSerializer);

			if (UBlueprint* Blueprint = Cast<UBlueprint>(Object))
			{
				// Serialize may have dirtied the BP bytecode in some way
				FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
			}
		}
	}
}

void MobilikonAssetRenameManager::PerformAssetRename(TArray<FAssetRenameDataWithReferencers>& AssetsToRename) const
{
	const FText AssetRenameSlowTask = LOCTEXT("AssetRenameSlowTask", "Renaming Assets");
	GWarn->BeginSlowTask(AssetRenameSlowTask, true);

	/**
	* We need to collect and check those cause dependency graph is only
	* representing on-disk state and we want to support rename for in-memory
	* objects. It is only needed for string references as in memory references
	* for other objects are pointers, so renames doesn't apply to those.
	*/
	TArray<UPackage *> DirtyPackagesToCheckForStringReferences;

	FEditorFileUtils::GetDirtyWorldPackages(DirtyPackagesToCheckForStringReferences);
	FEditorFileUtils::GetDirtyContentPackages(DirtyPackagesToCheckForStringReferences);

	TArray<UPackage*> PackagesToSave;
	TArray<UPackage*> PotentialPackagesToDelete;
	for (int32 AssetIdx = 0; AssetIdx < AssetsToRename.Num(); ++AssetIdx)
	{
		GWarn->StatusUpdate(AssetIdx, AssetsToRename.Num(), AssetRenameSlowTask);

		FAssetRenameDataWithReferencers& RenameData = AssetsToRename[AssetIdx];

		if (RenameData.bRenameFailed)
		{
			// The rename failed at some earlier step, skip this asset
			continue;
		}

		UObject* Asset = RenameData.Asset.Get();

		if (!Asset)
		{
			// This asset was invalid or GCed before the rename could occur
			RenameData.bRenameFailed = true;
			continue;
		}

		FString OldAssetPath = Asset->GetPathName();

		ObjectTools::FPackageGroupName PGN;
		PGN.ObjectName = RenameData.NewName;
		PGN.GroupName = TEXT("");
		PGN.PackageName = RenameData.NewPackagePath / PGN.ObjectName;
		const bool bLeaveRedirector = RenameData.bCreateRedirector;

		UPackage* OldPackage = Asset->GetOutermost();
		bool bOldPackageAddedToRootSet = false;
		if (!bLeaveRedirector && !OldPackage->IsRooted())
		{
			bOldPackageAddedToRootSet = true;
			OldPackage->AddToRoot();
		}

		TSet<UPackage*> ObjectsUserRefusedToFullyLoad;
		FText ErrorMessage;
		if (ObjectTools::RenameSingleObject(Asset, PGN, ObjectsUserRefusedToFullyLoad, ErrorMessage, NULL, bLeaveRedirector))
		{
			PackagesToSave.AddUnique(Asset->GetOutermost());

			// Automatically save renamed assets
			if (bLeaveRedirector)
			{
				PackagesToSave.AddUnique(OldPackage);
			}
			else if (bOldPackageAddedToRootSet)
			{
				// Since we did not leave a redirector and the old package wasnt already rooted, attempt to delete it when we are done. 
				PotentialPackagesToDelete.AddUnique(OldPackage);
			}
		}
		else
		{
			// No need to keep the old package rooted, the asset was never renamed out of it
			if (bOldPackageAddedToRootSet)
			{
				OldPackage->RemoveFromRoot();
			}

			// Mark the rename as a failure to report it later
			RenameData.bRenameFailed = true;
			RenameData.FailureReason = ErrorMessage;
		}

		TArray<UPackage *> PackagesToCheck(DirtyPackagesToCheckForStringReferences);

		for (auto PackageNameIt = RenameData.ReferencingPackageNames.CreateConstIterator(); PackageNameIt; ++PackageNameIt)
		{
			UPackage* PackageToCheck = FindPackage(NULL, *PackageNameIt->ToString());
			if (PackageToCheck)
			{
				PackagesToCheck.Add(PackageToCheck);
			}
		}

		TMap<FSoftObjectPath, FSoftObjectPath> RedirectorMap;
		RedirectorMap.Add(OldAssetPath, Asset->GetPathName());

		if (UBlueprint* Blueprint = Cast<UBlueprint>(Asset))
		{
			// Add redirect for class and default as well
			RedirectorMap.Add(FString::Printf(TEXT("%s_C"), *OldAssetPath), FString::Printf(TEXT("%s_C"), *Asset->GetPathName()));
			RedirectorMap.Add(FString::Printf(TEXT("Default__%s_C"), *OldAssetPath), FString::Printf(TEXT("Default__%s_C"), *Asset->GetPathName()));
		}

		
		RenameReferencingStringAssetReferences(PackagesToCheck, RedirectorMap);
	}

	GWarn->EndSlowTask();

	// Save all renamed assets and any redirectors that were left behind
	if (PackagesToSave.Num() > 0)
	{
		const bool bCheckDirty = false;
		const bool bPromptToSave = false;
		const bool bAlreadyCheckedOut = true;
		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, bCheckDirty, bPromptToSave, nullptr, bAlreadyCheckedOut);

		ISourceControlModule::Get().QueueStatusUpdate(PackagesToSave);
	}

	//// Now branch the files in source control if possible
	//for (const auto& AssetToRename : AssetsToRename)
	//{
	//	// If something went wrong when saving and the new asset does not exist on disk, don't branch it
	//	// as it will just create a copy and any attempt to load it will result in crashes.
	//	if (FPackageName::DoesPackageExist(AssetToRename.Asset->GetOutermost()->GetFName().ToString()))
	//	{
	//		SourceControlHelpers::BranchPackage(AssetToRename.Asset->GetOutermost(), FindPackage(nullptr, *AssetToRename.OriginalAssetPath));
	//	}
	//}

	// Clean up all packages that were left empty
	if (PotentialPackagesToDelete.Num() > 0)
	{
		for (auto PackageIt = PotentialPackagesToDelete.CreateConstIterator(); PackageIt; ++PackageIt)
		{
			(*PackageIt)->RemoveFromRoot();
		}

		//ObjectTools::CleanupAfterSuccessfulDelete(PotentialPackagesToDelete);
	}
}

void MobilikonAssetRenameManager::SaveReferencingPackages(const TArray<UPackage*>& ReferencingPackagesToSave) const
{
	if (ReferencingPackagesToSave.Num() > 0)
	{
		const bool bCheckDirty = false;
		const bool bPromptToSave = false;
		FEditorFileUtils::PromptForCheckoutAndSave(ReferencingPackagesToSave, bCheckDirty, bPromptToSave);

		ISourceControlModule::Get().QueueStatusUpdate(ReferencingPackagesToSave);
	}
}

void MobilikonAssetRenameManager::ReportFailures(const TArray<FAssetRenameDataWithReferencers>& AssetsToRename) const
{
	TArray<FText> FailedRenames;
	for (auto RenameIt = AssetsToRename.CreateConstIterator(); RenameIt; ++RenameIt)
	{
		const FAssetRenameDataWithReferencers& RenameData = *RenameIt;
		if (RenameData.bRenameFailed)
		{
			UObject* Asset = RenameData.Asset.Get();
			if (Asset)
			{
				FFormatNamedArguments Args;
				Args.Add(TEXT("FailureReason"), RenameData.FailureReason);
				Args.Add(TEXT("AssetName"), FText::FromString(Asset->GetOutermost()->GetName()));

				FailedRenames.Add(FText::Format(LOCTEXT("AssetRenameFailure", "{AssetName} - {FailureReason}"), Args));
			}
			else
			{
				FailedRenames.Add(LOCTEXT("InvalidAssetText", "Invalid Asset"));
			}
		}
	}

	//if (FailedRenames.Num() > 0)
	//{
	//	SRenameFailures::OpenRenameFailuresDialog(FailedRenames);
	//}
}

#undef LOCTEXT_NAMESPACE
