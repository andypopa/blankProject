// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Pkg.h"

#include "AsyncTaskDownloadSig.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDownloadSigDelegate, FString, String);

UCLASS()
class BLANKPROJECT_API UAsyncTaskDownloadSig : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable)
		static UAsyncTaskDownloadSig* DownloadSig(UPakAsset* PakAsset);
	UFUNCTION(BlueprintCallable, Category = "Async Task Download Sig", Meta = (DisplayName = "Check if Sig exists"))
		static bool CheckIfSigExists(UPakAsset* PakAsset);
public:
	UPROPERTY(BlueprintAssignable)
		FDownloadSigDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
		FDownloadSigDelegate OnFail;
public:
	static FString PackageFolder();
	void Start();
	UPakAsset* PakAsset;
private:
	void HandleSigRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
};
