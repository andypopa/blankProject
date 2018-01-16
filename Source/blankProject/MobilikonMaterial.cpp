// Fill out your copyright notice in the Description page of Project Settings.
#include "MobilikonMaterial.h"

UMobilikonMaterial* UMobilikonMaterial::CreateMaterial(FString _Id, FString _Name, FString Description, FString DiffuseMap, FString NormalMap, FString OcclusionMap, FString HeightMap, FString SpecularMap, FString _Pak, FString _Sig, bool& IsValid)
{
	IsValid = false;
	UMobilikonMaterial *Object = NewObject<UMobilikonMaterial>();

	if (!Object) {
		UE_LOG(LogTemp, Error, TEXT("[UMobilikonMaterial::CreateMaterial] Could not be created"));
		return NULL;
	}
	if (!Object->IsValidLowLevel()) {
		UE_LOG(LogTemp, Error, TEXT("[UMobilikonMaterial::CreateMaterial] Created object is not valid"));
		return NULL;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UMobilikonMaterial::CreateMaterial] An instance is created successfully"));

	Object->Id = FString(_Id);
	Object->Name = FString(_Name);
	Object->Description = FString(Description);
	Object->DiffuseMap = FString(DiffuseMap);
	Object->NormalMap = FString(NormalMap);
	Object->OcclusionMap = FString(OcclusionMap);
	Object->HeightMap = FString(HeightMap);
	Object->SpecularMap = FString(SpecularMap);
	Object->Pak = FString(_Pak);
	Object->Sig = FString(_Sig);

//    Object->ParentMaterialParticle = Object->GetParentMaterialParticle();
//    Object->ParentMaterial = Object->GetParentMaterial();
//    Object->DynamicMaterial = Object->CreateDynamicMaterialInstance();

	//Object->ConfigureDynamicMaterialInstance();
	IsValid = true;
	return Object;
}

//FString UMobilikonMaterial::GetParentMaterialParticle()
//{
//    auto particle = FString();
//    if (!this->DiffuseMap.IsEmpty()) {
//        particle += "-BaseColor";
//    }
//    if (!this->SpecularMap.IsEmpty()) {
//        particle += "-Specular";
//    }
//    if (!this->HeightMap.IsEmpty()) {
//        particle += "-Roughness";
//    }
//    if (!this->NormalMap.IsEmpty()) {
//        particle += "-Normal";
//    }
//    if (!this->OcclusionMap.IsEmpty()) {
//        particle += "-AmbientOcclusion";
//    }
//    return particle;
//}

//UMaterialInterface* UMobilikonMaterial::GetParentMaterial()
//{
//    //Material'/Game/DynamicMaterialParent-BaseColor-Normal.DynamicMaterialParent-BaseColor-Normal'
//    auto particle = this->ParentMaterialParticle;
//    auto path = FString("/Game/DynamicMaterialParent");
//    path += particle;
//    path += FString(".");
//    path += "DynamicMaterialParent";
//    path += particle;
//
//    UMaterialInterface* material = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, *path));
//
//    return material;
//}

//UMaterialInstanceDynamic* UMobilikonMaterial::CreateDynamicMaterialInstance()
//{
//    return UMaterialInstanceDynamic::Create(this->ParentMaterial, NULL);
//
//}

//UMaterialInstanceDynamic* UMobilikonMaterial::ConfigureDynamicMaterialInstance()
//{
//	auto parentMaterialName = this->ParentMaterialParticle;
//	auto dynamicMaterial = this->DynamicMaterial;
//
//	//THIS WILL ALL GO IN ONSUCCESS CALLBACK
//	if (parentMaterialName.Contains(FString("-BaseColor")))
//	{
//		//SHOULD DOWNLOAD HERE
//		dynamicMaterial->SetTextureParameterValue("BaseColor", this->DiffuseMapTexture);
//	}
//	if (parentMaterialName.Contains(FString("-Specular")))
//	{
//		dynamicMaterial->SetTextureParameterValue("Specular", this->SpecularMapTexture);
//	}
//	if (parentMaterialName.Contains(FString("-Roughness")))
//	{
//		dynamicMaterial->SetTextureParameterValue("Roughness", this->HeightMapTexture);
//	}
//	if (parentMaterialName.Contains(FString("-Normal")))
//	{
//		dynamicMaterial->SetTextureParameterValue("Normal", this->NormalMapTexture);
//	}
//	if (parentMaterialName.Contains(FString("-AmbientOcclusion")))
//	{
//		dynamicMaterial->SetTextureParameterValue("AmbientOcclusion", this->OcclusionMapTexture);
//	}
//	return dynamicMaterial;
//}

//void UMobilikonMaterial::DownloadTexture(FString url, FString textureParamaterName)
//{
//	UAsyncTaskDownloadImage* task = UAsyncTaskDownloadImage::DownloadImage(url);
//	
//}
