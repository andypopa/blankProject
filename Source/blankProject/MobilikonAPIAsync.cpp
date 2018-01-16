// Fill out your copyright notice in the Description page of Project Settings.
#include "MobilikonAPIAsync.h"
#include "Interfaces/IHttpRequest.h"
#include "HttpModule.h"
#include "CoreMinimal.h"
#include "AsyncTaskDownloadString.h"
#include "Json.h"
#include "JsonObject.h"
#include "Pkg.h"
#include "MobilikonMaterial.h"

const FString UMobilikonAPIAsync::Base = FString("http://localhost:1212");

UMobilikonAPIAsync::UMobilikonAPIAsync(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		AddToRoot();
	}
}

UAsyncTaskDownloadString* UMobilikonAPIAsync::GetPackages()
{
	FString url = UMobilikonAPIAsync::Base;
	url += TEXT("/ue/pkgs");
	UAsyncTaskDownloadString* DownloadTask = NewObject<UAsyncTaskDownloadString>();
	DownloadTask->Start(url);

	return DownloadTask;
}

UAsyncTaskDownloadString* UMobilikonAPIAsync::GetMaterials()
{
	FString url = UMobilikonAPIAsync::Base;
	url += TEXT("/ue/materials");
	UAsyncTaskDownloadString* DownloadTask = NewObject<UAsyncTaskDownloadString>();
	DownloadTask->Start(url);

	return DownloadTask;
}

TArray<UPkg*> UMobilikonAPIAsync::DeserializePkgs(FString json) {
	TArray<UPkg*> result = TArray<UPkg*>();

	TSharedPtr<FJsonObject> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(json);

	bool deserializationResult = FJsonSerializer::Deserialize(JsonReader, JsonParsed);
	if (!deserializationResult)
	{
		UE_LOG(LogTemp, Error, TEXT("FJsonSerializer: FAILED TO DESERIALIZE"));
	}
	auto data = JsonParsed->GetArrayField(FString("data"));
	for (int i = 0; i < data.Num(); ++i)
	{
		TSharedPtr<FJsonObject> item = data[i]->AsObject();
		FString id = item->GetStringField(FString("_id"));
		FString name = item->GetStringField(FString("name"));
		FString description = item->GetStringField(FString("description"));
		FString price = item->GetStringField(FString("price"));
		FString thumb = item->GetStringField(FString("thumb"));
		FString video = item->GetStringField(FString("video"));
		FString pak = item->GetStringField(FString("pak"));
		FString sig = item->GetStringField(FString("sig"));

		TArray<UMaterialSlot*> materialSlots;
		
		TArray<TSharedPtr<FJsonValue>> materialsJson = item->GetArrayField(FString("materials"));
		for (int c = 0; c < materialsJson.Num(); ++c) {
			TSharedPtr<FJsonObject> materialSlotJson = materialsJson[c]->AsObject();

			FString meshName = materialSlotJson->GetStringField(FString("meshName"));
			FString meshThumb = materialSlotJson->GetStringField(FString("meshThumb"));
			UMobilikonMaterial* material = UMobilikonAPIAsync::DeserializeMaterial(materialSlotJson->GetObjectField("material"));

			bool isValidMaterialSlot;
			UMaterialSlot* materialSlot = UMaterialSlot::CreateMaterialSlot(meshName, meshThumb, material, isValidMaterialSlot);
			materialSlots.Add(materialSlot);
		}

		bool isValid;
		result.Add(UPkg::CreatePkg(id, name, description, price, thumb, video, pak, sig, materialSlots, isValid));
	}

	return result;
}

TArray<UMobilikonMaterial*> UMobilikonAPIAsync::DeserializeMaterials(FString json) {
	TArray<UMobilikonMaterial*> result = TArray<UMobilikonMaterial*>();

	TSharedPtr<FJsonObject> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(json);

	bool deserializationResult = FJsonSerializer::Deserialize(JsonReader, JsonParsed);
	if (!deserializationResult)
	{
		UE_LOG(LogTemp, Error, TEXT("FJsonSerializer: FAILED TO DESERIALIZE"));
	}
	auto data = JsonParsed->GetArrayField(FString("data"));
	for (int i = 0; i < data.Num(); ++i)
	{
		TSharedPtr<FJsonObject> item = data[i]->AsObject();
		result.Add(UMobilikonAPIAsync::DeserializeMaterial(item));
		/*FString id = item->GetStringField(FString("_id"));
		FString name = item->GetStringField(FString("name"));
		FString description = item->GetStringField(FString("description"));
		FString diffuseMap = item->GetStringField(FString("diffuseMap"));
		FString normalMap = item->GetStringField(FString("normalMap"));
		FString occlusionMap = item->GetStringField(FString("occlusionMap"));
		FString heightMap = item->GetStringField(FString("heightMap"));
		FString specularMap = item->GetStringField(FString("specularMap"));
		bool isValid;
		result.Add(UMobilikonMaterial::CreateMaterial(id, name, description, diffuseMap, normalMap, occlusionMap, heightMap, specularMap, isValid));*/
	}

	return result;
}

UMobilikonMaterial* UMobilikonAPIAsync::DeserializeMaterial(const TSharedPtr<FJsonObject> item) {
	FString id = item->GetStringField(FString("_id"));
	FString name = item->GetStringField(FString("name"));
	FString description = item->GetStringField(FString("description"));
	FString diffuseMap = item->GetStringField(FString("diffuseMap"));
	FString normalMap = item->GetStringField(FString("normalMap"));
	FString occlusionMap = item->GetStringField(FString("occlusionMap"));
	FString heightMap = item->GetStringField(FString("heightMap"));
	FString specularMap = item->GetStringField(FString("specularMap"));
	FString pak = item->GetStringField(FString("pak"));
	FString sig = item->GetStringField(FString("sig"));
	bool isValid;
	return UMobilikonMaterial::CreateMaterial(id, name, description, diffuseMap, normalMap, occlusionMap, heightMap, specularMap, pak, sig, isValid);
}
