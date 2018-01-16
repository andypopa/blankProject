#pragma once

#include "Object.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Runtime/Online/HTTP/Public/HttpManager.h"
#include "JsonDownloader.generated.h"

DECLARE_DELEGATE_OneParam(FDirPackageRecursiveDownloader, FString);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FJsonDownloader_OnDownloadComplete, FString, name, FString, pak, FString, thumb);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FJsonDownloader_OnDownloadError, FString, packageName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FJsonDownloader_OnDownloadProgress, FString, packageName, int32, progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FJsonDownloader_OnUpdateCheckCompleted, FString, packageName, bool, hasUpdate);

UCLASS(Blueprintable, BlueprintType)
class UJsonDownloader : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Json Downloader", Meta = (DisplayName = "Get A Json Downloader"))
		static UJsonDownloader* GetJsonDownloader(FString PackageName, FString URL, bool& IsValid);

	UPROPERTY(BlueprintAssignable, Category = "Json Downloader")
		FJsonDownloader_OnDownloadComplete OnPackageDownloadCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Json Downloader")
		FJsonDownloader_OnDownloadError OnJsonDownloaderror;

	UPROPERTY(BlueprintAssignable, Category = "Json Downloader")
		FJsonDownloader_OnDownloadProgress OnPackageDownloadProgress;

	UPROPERTY(BlueprintAssignable, Category = "Json Downloader")
		FJsonDownloader_OnUpdateCheckCompleted OnUpdateCheckCompleted;

	UFUNCTION(BlueprintCallable, Category = "Json Downloader", Meta = (DisplayName = "Download Package"))
		void DownloadPackage();

	UFUNCTION(BlueprintCallable, Category = "Json Downloader", Meta = (DisplayName = "Cancel Download"))
		void CancelDownload();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Json Downloader", Meta = (DisplayName = "Package Name"))
		FString OriginalPackageName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Json Downloader", Meta = (DisplayName = "Server URL"))
		FString OriginalURL;


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