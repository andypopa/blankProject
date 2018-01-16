// Fill out your copyright notice in the Description page of Project Settings.
#include "CreateProductWidget.h"
#include "Engine.h"

UUserWidget* UCreateProductWidget::CreateProductWidget(UPkg* pkg, UPkg*& pkgRef, FString& pkgName, FString& pkgThumb)
{
	UE_LOG(LogTemp, Warning, TEXT("[CreateProductWidget::CreateProductWidget] Whoppy! %s"), *pkg->Id);
	auto className = TEXT("UserWidget'/Game/Add_Product_Menu/Single_product_holder.Single_product_holder_C'");
	UClass* MyBpClass = FindObject<UClass>(nullptr, className, false);
	if (MyBpClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UAsyncTaskCreateProductWidget::OnDownloadImageSuccess] Class not found %s"), *className);
		return nullptr;
	}
	
	UUserWidget* widgetInstance = CreateWidget<UUserWidget>(GEngine->GameViewport->GetWorld(), MyBpClass);

	pkgRef = pkg;
	pkgName = pkg->Name;
	pkgThumb = pkg->Thumb;
	return widgetInstance;
}