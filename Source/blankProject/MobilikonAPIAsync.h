// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpResponse.h"
#include "AsyncTaskDownloadString.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Pkg.h"
#include "JsonObject.h"

#include "MobilikonAPIAsync.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDownloadStringDelegate, FString, String);

UCLASS()
class BLANKPROJECT_API UMobilikonAPIAsync : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
        static const FString Base;
	UFUNCTION(BlueprintCallable, Category = "Mobilikon API Async", Meta = (DisplayName = "Get Packages Async"))
		static UAsyncTaskDownloadString* GetPackages();
	UFUNCTION(BlueprintCallable, Category = "Mobilikon API Async", Meta = (DisplayName = "Get Materials Async"))
		static UAsyncTaskDownloadString* GetMaterials();
	UFUNCTION(BlueprintPure, Category = "Mobilikon API", Meta = (DisplayName = "Deserialize Pkgs"))
		static TArray<UPkg*> DeserializePkgs(FString json);
	UFUNCTION(BlueprintPure, Category = "Mobilikon API", Meta = (DisplayName = "Deserialize Materials"))
		static TArray<UMobilikonMaterial*> DeserializeMaterials(FString json);
//    UFUNCTION(BlueprintPure, Category = "Mobilikon API", Meta = (DisplayName = "Deserialize Material"))
		static UMobilikonMaterial* DeserializeMaterial(const TSharedPtr<FJsonObject> item);
	UPROPERTY(BlueprintAssignable)
		FDownloadStringDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
		FDownloadStringDelegate OnFail;
};
