// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "AsyncTaskDownloadString.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDownloadStringDelegate, FString, String);

UCLASS()
class BLANKPROJECT_API UAsyncTaskDownloadString : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable)
		static UAsyncTaskDownloadString* DownloadString(FString URL);

public:

	UPROPERTY(BlueprintAssignable)
		FDownloadStringDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
		FDownloadStringDelegate OnFail;

public:
	void Start(FString URL);

private:
	void HandleStringRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
};
