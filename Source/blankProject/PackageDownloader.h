#pragma once

#include "Object.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Runtime/Online/HTTP/Public/HttpManager.h"
#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"

#include "PackageDownloader.generated.h"

DECLARE_DELEGATE_OneParam(FDirPackageRecursiveDownloader, FString);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPackageDownloader_OnDownloadComplete, FString, mapName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPackageDownloader_OnDownloadError, FString, mapName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPackageDownloader_OnDownloadProgress, FString, mapName, int32, progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPackageDownloader_OnUpdateCheckCompleted, FString, mapName, bool, hasUpdate);

UCLASS(Blueprintable, BlueprintType)
class UPackageDownloader : public UObject
{
	GENERATED_BODY()

public:
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	UFUNCTION(BlueprintPure, Category = "Package Downloader", Meta = (DisplayName = "Get A Package Downloader"))
		static UPackageDownloader* GetPackageDownloader(FString PackageName, FString URL, bool& IsValid);

	UFUNCTION(BlueprintPure, Category = "Package Downloader", Meta = (DisplayName = "Downloaded Packages List"))
		static TArray<FString> DownloadedPackagesList();
	UFUNCTION(BlueprintPure, Category = "Package Downloader", Meta = (DisplayName = "Delete Package File"))
		static void DeletePackageFile(FString PackageName, bool &isDeleted);

	UFUNCTION(BlueprintPure, Category = "Package Downloader", Meta = (DisplayName = "Is Package Already Downloaded"))
		static void IsPackageDownloaded(FString PackageName, bool &isDownloaded);

	UFUNCTION(BlueprintPure, Category = "Package Downloader", Meta = (DisplayName = "Is Package Already Mounted"))
		static void IsPackageMounted(FString PackageName, bool &isMounted);

	UFUNCTION(BlueprintCallable, Category = "Package Downloader", Meta = (DisplayName = "Mount Package"))
		TArray<UStaticMesh*> MountPackage(FString PackageName, FString MountFolder, bool& isMounted);
	
	UFUNCTION(BlueprintPure, Category = "Package Downloader")
		static bool HotfixPakFile(FString Path);

	UPROPERTY(BlueprintAssignable, Category = "Package Downloader")
		FPackageDownloader_OnDownloadComplete OnPackageDownloadCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Package Downloader")
		FPackageDownloader_OnDownloadError OnPackageDownloadError;

	UPROPERTY(BlueprintAssignable, Category = "Package Downloader")
		FPackageDownloader_OnDownloadProgress OnPackageDownloadProgress;

	UPROPERTY(BlueprintAssignable, Category = "Package Downloader")
		FPackageDownloader_OnUpdateCheckCompleted OnUpdateCheckCompleted;

	UFUNCTION(BlueprintCallable, Category = "Package Downloader", Meta = (DisplayName = "Does Package Have Update"))
		void CheckIfPackageHasUpdate();

	UFUNCTION(BlueprintCallable, Category = "Package Downloader", Meta = (DisplayName = "Download Package"))
		void DownloadPackage();

	UFUNCTION(BlueprintCallable, Category = "Package Downloader", Meta = (DisplayName = "Cancel Download"))
		void CancelDownload();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Package Downloader", Meta = (DisplayName = "Package Name"))
		FString OriginalPackageName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Package Downloader", Meta = (DisplayName = "Server URL"))
		FString OriginalURL;

	static FString PackageFolder();
	//;
	static bool CreatePackageFolder();

	//FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	void OnAssetAdded(const FAssetData& AssetData);
private:

	void HttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HttpDownloadComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HttpRequestProgress(FHttpRequestPtr Request, int32 bytesSent, int32 bytesReceived);

	void UpdateCheckHttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	int32 RequestSize;
	FString RequestUrl;

	TSharedPtr<IHttpRequest> UpdateRequest;
	TSharedPtr<IHttpRequest> DownloadRequest;
};