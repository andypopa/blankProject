// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PakAsset.h"

#include "AsyncTaskDownloadPak.generated.h"

DECLARE_DELEGATE_OneParam(FDirPackageRecursiveDownloader, FString);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDownloadPakDelegate, FString, String);

UCLASS()
class BLANKPROJECT_API UAsyncTaskDownloadPak : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable)
		static UAsyncTaskDownloadPak* DownloadPak(UPakAsset* PakAsset);

	UFUNCTION(BlueprintCallable, Category = "Async Task Download Pak", Meta = (DisplayName = "Mount Package"))
		static TArray<UStaticMesh*> MountPackage(UPakAsset* PakAsset, bool& isMounted);
	UFUNCTION(BlueprintCallable, Category = "Async Task Download Pak", Meta = (DisplayName = "Mount Material Package"))
		static TArray<UMaterial*> MountMaterialPackage(UPakAsset* PakAsset, bool& isMounted);

	UFUNCTION(BlueprintCallable, Category = "Async Task Download Pak", Meta = (DisplayName = "Check if pak exists"))
		static bool CheckIfPakExists(UPakAsset* PakAsset);

public:
	UPROPERTY(BlueprintAssignable)
		FDownloadPakDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
		FDownloadPakDelegate OnFail;
public:
	static const ANSICHAR* GetKey();
	static bool CreatePackageFolder();
	static FString PackageFolder();
	void Start();
	UPakAsset* PakAsset;
private:
	void HandlePakRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
};
