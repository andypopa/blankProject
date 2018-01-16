// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "MaterialSlot.h"

#include "PakAsset.h"
#include "Pkg.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UPkg : public UPakAsset
{
GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Pkg", Meta = (DisplayName = "Create Pkg"))
	static UPkg* CreatePkg(FString _Id, FString _Name, FString Description, FString Price, FString Thumb, FString Video, FString _Pak, FString _Sig, TArray<UMaterialSlot*> MaterialSlots, bool& IsValid);
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pkg")
	//FString Id;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pkg")
	//FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pkg")
	FString Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pkg")
	FString Price;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pkg")
	FString Thumb;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pkg")
	FString Video;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pkg")
	//FString Pak;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pkg")
	//FString Sig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pkg")
	TArray<UMaterialSlot*> MaterialSlots;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pkg")
	UTexture2DDynamic* ThumbTexture;
	UFUNCTION(BlueprintCallable, Category = "Pkg")
	UMobilikonMaterial* GetMaterialForMesh(UStaticMesh* StaticMesh);
};