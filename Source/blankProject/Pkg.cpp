// Fill out your copyright notice in the Description page of Project Settings.
#include "Pkg.h"
#include "MaterialSlot.h"
#include "Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"

UPkg* UPkg::CreatePkg(FString _Id, FString _Name, FString Description, FString Price, FString Thumb, FString Video, FString _Pak, FString _Sig, TArray<UMaterialSlot*> MaterialSlots, bool& IsValid)
{
	IsValid = false;
	UPkg *Object = NewObject<UPkg>();

	if (!Object) {
		UE_LOG(LogTemp, Error, TEXT("[UPkg::CreatePkg] Could not be created"));
		return NULL;
	}
	if (!Object->IsValidLowLevel()) {
		UE_LOG(LogTemp, Error, TEXT("[UPkg::CreatePkg] Created object is not valid"));
		return NULL;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UPkg::CreatePkg] An instance is created successfully"));

	Object->Id = FString(_Id);
	Object->Name = FString(_Name);
	Object->Description = FString(Description);
	Object->Price = FString(Price);
	Object->Thumb = FString(Thumb);
	Object->Video = FString(Video);
	Object->Pak = FString(_Pak);
	Object->Sig = FString(_Sig);
	Object->MaterialSlots = MaterialSlots;

	IsValid = true;
	return Object;
}

UMobilikonMaterial* UPkg::GetMaterialForMesh(UStaticMesh* StaticMesh) {
	//auto name = UKismetSystemLibrary::GetDisplayName(dynamic_cast<UObject*>(StaticMesh));
    auto name = StaticMesh->GetName();
	for (auto materialSlot : this->MaterialSlots)
	{
		auto isMatch = name.EndsWith(materialSlot->UE4MeshName);
		if (isMatch)
		{
			return materialSlot->Material;
		}
	}
	return nullptr;
}
