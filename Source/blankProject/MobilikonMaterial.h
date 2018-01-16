// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "Materials/MaterialInterface.h"
#include "Runtime/UMG/Public/Blueprint/AsyncTaskDownloadImage.h"
#include "PakAsset.h"

#include "MobilikonMaterial.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMobilikonMaterial_OnDownloadTextureComplete, FString, textureParameterName, UTexture*, texture);

UCLASS(Blueprintable, BlueprintType)
class UMobilikonMaterial : public UPakAsset
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Mobilikon Material", Meta = (DisplayName = "Create Material"))
		static UMobilikonMaterial* CreateMaterial (FString _Id, FString _Name, FString Description, FString DiffuseMap, FString NormalMap, FString OcclusionMap, FString HeightMap, FString SpecularMap, FString _Pak, FString _Sig, bool& IsValid);
//    UFUNCTION(BlueprintPure, Category = "Mobilikon Material", Meta = (DisplayName = "Get Parent Material Particle"))
//        FString GetParentMaterialParticle();
//    UFUNCTION(BlueprintPure, Category = "Mobilikon Material", Meta = (DisplayName = "Get Parent Material"))
//        UMaterialInterface* GetParentMaterial();
//    UFUNCTION(BlueprintPure, Category = "Mobilikon Material", Meta = (DisplayName = "Create Dynamic Material Instance"))
//        UMaterialInstanceDynamic* CreateDynamicMaterialInstance();
	//UFUNCTION(BlueprintPure, Category = "Mobilikon Material", Meta = (DisplayName = "Configure Dynamic Material Instance"))
		//UMaterialInstanceDynamic* ConfigureDynamicMaterialInstance();
	//FIX BP FN DECL: EXPOSE FN
	//UFUNCTION(BlueprintPure, Category = "Mobilikon Material", Meta = (DisplayName = "Download Texture"))
		void DownloadTexture(FString url, FString textureParamaterName);

	//OnSuccess Delegate
//    UPROPERTY(BlueprintAssignable, Category = "Mobilikon Material")
//        FMobilikonMaterial_OnDownloadTextureComplete OnDownloadTextureComplete;

	//OnFail Delegate
	//TODO

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
	//	FString Id;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
	//	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
		FString Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
		FString DiffuseMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
		FString NormalMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
		FString OcclusionMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
		FString HeightMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
		FString SpecularMap;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
	//	FString Pak;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
	//	FString Sig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
		UTexture* DiffuseMapTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
		UTexture* NormalMapTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
		UTexture* OcclusionMapTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
		UTexture* HeightMapTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
		UTexture* SpecularMapTexture;

//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
//        FString ParentMaterialParticle;

//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
//        UMaterialInterface* ParentMaterial;
//
//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
//        UMaterialInstanceDynamic* DynamicMaterial;
//
//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobilikon Material")
//        bool bConfigured = false;
};
