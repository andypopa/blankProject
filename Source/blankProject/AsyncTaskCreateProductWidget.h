// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Pkg.h"
#include "Runtime/UMG/Public/Blueprint/AsyncTaskDownloadImage.h"
#include "Engine/Texture2DDynamic.h"

#include "AsyncTaskCreateProductWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateProductWidgetDelegate, FString, String);

UCLASS()
class BLANKPROJECT_API UAsyncTaskCreateProductWidget : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable)
		static UAsyncTaskCreateProductWidget* CreateProductWidget(UPkg* pkg);

public:
	UPROPERTY(BlueprintReadOnly)
		UPkg* Pkg;
	UPROPERTY(BlueprintAssignable)
		FCreateProductWidgetDelegate OnSuccess;
	UPROPERTY(BlueprintAssignable)
		FCreateProductWidgetDelegate OnFail;
	UFUNCTION()
		void OnDownloadImageSuccess(UTexture2DDynamic* Texture);
	UFUNCTION()
		void OnDownloadImageFail(UTexture2DDynamic* Texture);
public:
	void Start(FString URL);
	static void OnStatic(UTexture2DDynamic* Texture);
private:
	void HandleStringRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

};
