// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Commandlets/Commandlet.h"
#include "CoreMinimal.h"
#include "MaterialCreatorCommandlet.generated.h"
/**
 * 
 */
UCLASS()
class BLANKPROJECT_API UMaterialCreatorCommandlet : public UCommandlet
{

	GENERATED_BODY()
public:

	/** Default constructor. */
	UMaterialCreatorCommandlet();

public:

	//~ UCommandlet interface

	virtual int32 Main(const FString& Params) override;


};