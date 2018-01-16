// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "MobilikonMaterial.h"

#include "MaterialSlot.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UMaterialSlot : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Material Slot", Meta = (DisplayName = "Create Material Slot"))
		static UMaterialSlot* CreateMaterialSlot(FString MeshName, FString MeshThumb, UMobilikonMaterial* Material, bool& IsValid);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Slot")
		FString MeshName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Slot")
		FString UE4MeshName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Slot")
		FString MeshThumb;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Slot")
		UMobilikonMaterial* Material;
};