// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Pkg.h"

#include "CreateProductWidget.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UCreateProductWidget : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Product Widget", Meta = (DisplayName = "Create Product Widget"))
		static UUserWidget* CreateProductWidget(UPkg* pkg, UPkg*& pkgRef, FString& pkgName, FString& pkgThumb);
};