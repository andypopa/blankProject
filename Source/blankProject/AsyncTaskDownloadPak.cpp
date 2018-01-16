#include "AsyncTaskDownloadPak.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "PlatformFilemanager.h"
#include "IPlatformFilePak.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#include "CoreDelegates.h"
#include "AssetRegistryModule.h"
#include "Engine/StreamableManager.h"
#include "ConstructorHelpers.h"
#include "Engine/Texture2D.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "Engine/StaticMesh.h"
#include "MobilikonMaterial.h"

//----------------------------------------------------------------------//
// UAsyncTaskDownloadPak
//----------------------------------------------------------------------//

UAsyncTaskDownloadPak::UAsyncTaskDownloadPak(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		AddToRoot();
	}
}

UAsyncTaskDownloadPak* UAsyncTaskDownloadPak::DownloadPak(UPakAsset* PakAsset)
{
	UAsyncTaskDownloadPak* DownloadTask = NewObject<UAsyncTaskDownloadPak>();
	DownloadTask->PakAsset = PakAsset;
	DownloadTask->Start();
	return DownloadTask;
}


bool UAsyncTaskDownloadPak::CheckIfPakExists(UPakAsset* PakAsset)
{
	FString Filename = PackageFolder() + "/" + PakAsset->Id + ".pak";
	if (FPaths::FileExists(Filename))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadPak::CheckIfPakExists] %s PAK EXISTS"), *PakAsset->Name);
		return true;
	}
	return false;
}

void UAsyncTaskDownloadPak::Start()
{
	//if .pak already exists
	FString Filename = PackageFolder() + "/" + this->PakAsset->Id + ".pak";
	if (FPaths::FileExists(Filename))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadPak::DownloadPackage] %s PAK ALREADY EXISTS. Broadcasting OnSuccess."), *this->PakAsset->Name);
		OnSuccess.Broadcast(this->PakAsset->Name);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadPak::DownloadPackage] %s Requesting headers for %s"), *this->PakAsset->Name, *this->PakAsset->Pak);

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(this->PakAsset->Pak);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UAsyncTaskDownloadPak::HandlePakRequest);
	if (!HttpRequest->ProcessRequest())
	{
		OnFail.Broadcast(this->PakAsset->Name);
	}
}

void UAsyncTaskDownloadPak::HandlePakRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
//#if !UE_SERVER
//	RemoveFromRoot();
//
//	if (bSucceeded && HttpResponse.IsValid() && HttpResponse->GetContentLength() > 0)
//	{
//		OnSuccess.Broadcast(HttpResponse->GetContentAsString());
//		return;
//	}
//
//	OnFail.Broadcast(FString("Failed"));
//#endif
	//auto RequestUrl = HttpRequest->GetURL();
	//if (!HttpResponse.IsValid()) {
	//	UE_LOG(LogTemp, Error, TEXT("[UAsyncTaskDownloadPak::HttpRequestComplete] Could not connect to %s"), *RequestUrl);
	//	OnFail.Broadcast(this->PakAsset->Name);
	//	return;
	//}

	//if (!bSucceeded) {
	//	UE_LOG(LogTemp, Error, TEXT("[UAsyncTaskDownloadPak::HttpRequestComplete] Could not connect to %s"), *RequestUrl);
	//	OnFail.Broadcast(this->PakAsset->Name);
	//	return;
	//}

	//UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadPak::HttpRequestComplete] %s Starting download of %s"), *this->PakAsset->Name, *RequestUrl);

	//// Finding size of the requested file
	//	int32 StatusCode = HttpResponse->GetResponseCode();
	//	if (StatusCode / 100 != 2)
	//	{
	//		UE_LOG(LogTemp, Error, TEXT("[UAsyncTaskDownloadPak::HttpRequestComplete] %s HTTP response %d, for %s"), *this->PakAsset->Name, StatusCode, *RequestUrl);
	//		OnFail.Broadcast(this->PakAsset->Name);
	//		return;
	//	}

	//	TArray<FString> headers = HttpResponse->GetAllHeaders();
	//	for (FString h : headers) {
	//		UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadPak::HttpRequestComplete] %s Header: %s"), *this->PakAsset->Name, *h);
	//	}

	//	//for (FString h : headers) {
	//	//	if (h.Contains("x-file-size", ESearchCase::IgnoreCase) || h.Contains("Content-Length", ESearchCase::IgnoreCase)) {
	//	//		FString left;
	//	//		FString right;
	//	//		h.Split(":", &left, &right, ESearchCase::IgnoreCase, ESearchDir::FromStart);
	//	//		if (right.Len())
	//	//		{
	//	//			RequestSize = FCString::Atoi(*right);
	//	//		}

	//	//		break;
	//	//	}
	//	//}

	//	HttpRequest.Reset();

	//HttpRequest.Reset();
	auto RequestUrl = HttpRequest->GetURL();

	if (!HttpResponse.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::HttpDownloadComplete] Could not connect to %s"), *RequestUrl);
		OnFail.Broadcast(this->PakAsset->Name);
		return;
	}

	if (!bSucceeded) {
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::HttpDownloadComplete] Could not connect to %s"), *RequestUrl);
		OnFail.Broadcast(this->PakAsset->Name);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::HttpDownloadComplete] Download completed for %s from %s"), *this->PakAsset->Name, *RequestUrl);

	int32 StatusCode = HttpResponse->GetResponseCode();
	if (StatusCode / 100 != 2)
	{
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::HttpDownloadComplete] %s HTTP response %d, for %s"), *this->PakAsset->Name, StatusCode, *RequestUrl);
		OnFail.Broadcast(this->PakAsset->Name);
		return;
	}

	TArray<FString> headers = HttpResponse->GetAllHeaders();
	for (FString h : headers) {
		UE_LOG(LogTemp, Warning, TEXT("UExpoSocket::HttpDownloadComplete] %s Header: %s"), *this->PakAsset->Name, *h);
	}

	const TArray<uint8>& Content = HttpResponse->GetContent();

	FString Filename = PackageFolder() + "/" + this->PakAsset->Id + ".pak";
	UE_LOG(LogTemp, Error, TEXT("sunt o conditie bulangie"));
	if (FFileHelper::SaveArrayToFile(Content, *Filename))
	{
		UE_LOG(LogTemp, Error, TEXT("sunt o fucntie bulangie"));
		OnSuccess.Broadcast(this->PakAsset->Name);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::HttpDownloadComplete] %s Could not write %s to disk "), *this->PakAsset->Name, *Filename);
		OnFail.Broadcast(this->PakAsset->Name);
	}
}

FString UAsyncTaskDownloadPak::PackageFolder()
{
	FString FileDir = FPaths::ProjectPersistentDownloadDir() + "/DownPaks/";
	FPaths::NormalizeDirectoryName(FileDir);
	return FString(FPaths::ConvertRelativePathToFull(FileDir));
}

TArray<UMaterial*> UAsyncTaskDownloadPak::MountMaterialPackage(UPakAsset* PakAsset, bool& isMounted)
{
	TArray<UMaterial*> result = TArray<UMaterial*>();

	UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadPak::MountMaterialPackage] Mounting package %s"), *PakAsset->Name);
	if (FCoreDelegates::GetPakEncryptionKeyDelegate().IsBound()) {
		UE_LOG(LogTemp, Warning, TEXT("PakEncryptionKeyDelegate Delegate Is Bound"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PakEncryptionKeyDelegate Delegate IS NOT BOUND. Binding to GetKey"));
		FCoreDelegates::GetPakEncryptionKeyDelegate().BindStatic(&UAsyncTaskDownloadPak::GetKey);
	}

	if (FCoreDelegates::GetPakEncryptionKeyDelegate().IsBound()) {
		UE_LOG(LogTemp, Warning, TEXT("PakEncryptionKeyDelegate Delegate Is Bound"));
	}

	if (FCoreDelegates::GetPakSigningKeysDelegate().IsBound()) {
		UE_LOG(LogTemp, Warning, TEXT("PakSigningKeysDelegate Is Bound"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PakSigningKeysDelegate Delegate IS NOT BOUND"));
	}

	FString PackageFile = PackageFolder() + "/" + PakAsset->Id + ".pak";
	isMounted = false;
	bool bSuccessfulInitialization = false;

	IPlatformFile* LocalPlatformFile = &FPlatformFileManager::Get().GetPlatformFile();
	UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadPak::MountMaterialPackage] Mounting package %s"), *PakAsset->Name);

	if (LocalPlatformFile != nullptr)
	{
		IPlatformFile* PakPlatformFile = FPlatformFileManager::Get().GetPlatformFile(TEXT("PakFile"));
		if (PakPlatformFile != nullptr) bSuccessfulInitialization = true;
	}
	if (bSuccessfulInitialization)
	{
		const TCHAR* cmdLine = TEXT("");
		FPakPlatformFile* PakPlatform = new FPakPlatformFile();
		IPlatformFile* InnerPlatform = LocalPlatformFile;
		PakPlatform->Initialize(InnerPlatform, cmdLine);
		FPlatformFileManager::Get().SetPlatformFile(*PakPlatform);

		const TCHAR* PakAssetFile = *PackageFile;
		FPakFile PakFile(InnerPlatform, *PackageFile, true);
		if (!PakFile.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[UAsyncTaskDownloadPak::MountMaterialPackage] Invalid pak file %s"), *PakAsset->Name);
			return TArray<UMaterial*>();
		}

		FString MountPoint = FPaths::EngineContentDir() + PakAsset->Id;
		UE_LOG(LogTemp, Warning, TEXT("Mount point: %s"), *MountPoint);
		PakFile.SetMountPoint(*MountPoint);

		const int32 PakOrder = 0;

		if (!PakPlatform->Mount(*PackageFile, PakOrder, *MountPoint))
		{
			UE_LOG(LogTemp, Error, TEXT("[UAsyncTaskDownloadPak::MountMaterialPackage] Failed to mount package %s"), *MountPoint);
			return result;
		}

		isMounted = true;
		TArray<FString> FileList;
		PakFile.FindFilesAtPath(FileList, *PakFile.GetMountPoint(), true, false, true);
		for (int32 a = 0; a < FileList.Num(); a++) {
			UE_LOG(LogTemp, Error, TEXT("%s"), *FileList[a])
		}
		UObjectLibrary *ObjectLibrary = NewObject<UObjectLibrary>(UObjectLibrary::StaticClass());
		ObjectLibrary->UseWeakReferences(true);

		UE_LOG(LogTemp, Log, TEXT("Before load asset"));

		FString path = FString(TEXT("/Engine/") + PakAsset->Id);
		UE_LOG(LogTemp, Log, TEXT("%s"), *path);
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		TArray<FString> paths = TArray<FString>();
		paths.Add(path);

		AssetRegistry.ScanPathsSynchronous(paths, true);
		ObjectLibrary->LoadAssetDataFromPath(path);
		if (!ObjectLibrary->IsLibraryFullyLoaded()) {
			UE_LOG(LogTemp, Log, TEXT("Loading Object Library"));
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("Fully Loaded"));
		}
		ObjectLibrary->LoadAssetsFromAssetData();
		int32 count = ObjectLibrary->GetAssetDataCount();
		TArray<FAssetData> AssetDataList;
		ObjectLibrary->GetAssetDataList(AssetDataList);
		//return <UStaticMesh*>();

		FStreamableManager* Streamable = new FStreamableManager();
		UE_LOG(LogTemp, Log, TEXT("count:%d"), count);
		for (int32 x = 0; x < count; x++) {
			FAssetData AssetData = AssetDataList[x];
			FString Name = AssetData.ObjectPath.ToString();
			FString Class = AssetData.AssetClass.ToString();
			UE_LOG(LogTemp, Warning, TEXT("Name: %s"), *Name);
			UE_LOG(LogTemp, Warning, TEXT("Class: %s"), *Class);
			if (Class == FString("Texture2D")) {
				UE_LOG(LogTemp, Warning, TEXT("Class is Texture2D. Yielding: %s"), *Name);

				UE_LOG(LogTemp, Error, TEXT("Trying to load with StaticLoadObject"));
				UTexture2D* myTex = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *Name));
				if (!myTex) {
					UE_LOG(LogTemp, Error, TEXT("Trying to load with LoadObject"));
					myTex = LoadObject<UTexture2D>(NULL, *Name, NULL, LOAD_None, NULL);
				}
				if (!myTex) {
					UE_LOG(LogTemp, Error, TEXT("Trying to load with LoadSynchronous"));
					TAssetPtr<UTexture2D> BaseTex;
					FStringAssetReference AssetRef = AssetData.ToSoftObjectPath();
					BaseTex = Cast<UTexture2D>(Streamable->LoadSynchronous(AssetRef, true));
					myTex = BaseTex.Get();
				}
				UE_LOG(LogTemp, Error, TEXT("Texture2D address: %p"), &myTex);
			}
			if (Class == FString("Material"))
			{
				UE_LOG(LogTemp, Warning, TEXT("Class is Material. Yielding: %s"), *Name);
				UE_LOG(LogTemp, Error, TEXT("Trying to load with StaticLoadObject"));
//                UMaterial* myMaterial = Cast<UMaterial>(StaticLoadObject(UObject::StaticClass(), NULL, *Name));
                UMaterial* myMaterial = LoadObject<UMaterial>(nullptr, *Name);
//                if (!myMaterial) {
//                    UE_LOG(LogTemp, Error, TEXT("Trying to load with LoadObject"));
//                    myMaterial = LoadObject<UMaterial>(NULL, *Name, NULL, LOAD_None, NULL);
//                }
//                if (!myMaterial) {
//                    UE_LOG(LogTemp, Error, TEXT("Trying to load with LoadSynchronous"));
//                    TAssetPtr<UMaterial> BaseMaterial;
//                    FStringAssetReference AssetRef = AssetData.ToSoftObjectPath();
//                    BaseMaterial = Cast<UMaterial>(Streamable->LoadSynchronous(AssetRef, true));
//                    myMaterial = BaseMaterial.Get();
//                }

				result.Add(myMaterial);
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadPak::MountMaterialPackage] Mounted to %s. Returning result..."), *MountPoint);
#if WITH_EDITOR
		PakPlatform->Unmount(*PackageFile);
		ObjectLibrary->ClearLoaded();
#endif
		return result;
	}
	return result;
}

TArray<UStaticMesh*> UAsyncTaskDownloadPak::MountPackage(UPakAsset* PakAsset, bool& isMounted)
{
	TArray<UStaticMesh*> result = TArray<UStaticMesh*>();

	UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadPak::MountPackage] Mounting package %s"), *PakAsset->Name);
	if (FCoreDelegates::GetPakEncryptionKeyDelegate().IsBound()) {
		UE_LOG(LogTemp, Warning, TEXT("PakEncryptionKeyDelegate Delegate Is Bound"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PakEncryptionKeyDelegate Delegate IS NOT BOUND. Binding to GetKey"));
		FCoreDelegates::GetPakEncryptionKeyDelegate().BindStatic(&UAsyncTaskDownloadPak::GetKey);
	}

	if (FCoreDelegates::GetPakEncryptionKeyDelegate().IsBound()) {
		UE_LOG(LogTemp, Warning, TEXT("PakEncryptionKeyDelegate Delegate Is Bound"));
	}

	if (FCoreDelegates::GetPakSigningKeysDelegate().IsBound()) {
		UE_LOG(LogTemp, Warning, TEXT("PakSigningKeysDelegate Is Bound"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PakSigningKeysDelegate Delegate IS NOT BOUND"));
	}

	FString PackageFile = PackageFolder() + "/" + PakAsset->Id + ".pak";
	isMounted = false;
	bool bSuccessfulInitialization = false;

	IPlatformFile* LocalPlatformFile = &FPlatformFileManager::Get().GetPlatformFile();
	UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadPak::MountPackage] Mounting package %s"), *PakAsset->Name);

	if (LocalPlatformFile != nullptr)
	{
		IPlatformFile* PakPlatformFile = FPlatformFileManager::Get().GetPlatformFile(TEXT("PakFile"));
		if (PakPlatformFile != nullptr) bSuccessfulInitialization = true;
	}
	if (bSuccessfulInitialization)
	{
		const TCHAR* cmdLine = TEXT("");
		FPakPlatformFile* PakPlatform = new FPakPlatformFile();
		IPlatformFile* InnerPlatform = LocalPlatformFile;
		PakPlatform->Initialize(InnerPlatform, cmdLine);
		FPlatformFileManager::Get().SetPlatformFile(*PakPlatform);

		const TCHAR* PakAssetFile = *PackageFile;
		FPakFile PakFile(InnerPlatform, *PackageFile, true);
		if (!PakFile.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[UAsyncTaskDownloadPak::MountPackage] Invalid pak file %s"), *PakAsset->Name);
			return TArray<UStaticMesh*>();
		}

		//in editor should pe EngineContentDir
		FString MountPoint = FPaths::EngineContentDir() + PakAsset->Id;
		UE_LOG(LogTemp, Warning, TEXT("Mount point: %s"), *MountPoint);
		PakFile.SetMountPoint(*MountPoint);

		const int32 PakOrder = 0;

		//#if WITH_EDITOR
		//		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::MountPackage] Not mounting %s/%s in editor"), *MountPoint, *PackageFile);
		//#else
		if (!PakPlatform->Mount(*PackageFile, PakOrder, *MountPoint))
		{
			UE_LOG(LogTemp, Error, TEXT("[UAsyncTaskDownloadPak::MountPackage] Failed to mount package %s"), *MountPoint);
			return result;
		}
		//#endif

		isMounted = true;
		TArray<FString> FileList;
		PakFile.FindFilesAtPath(FileList, *PakFile.GetMountPoint(), true, false, true);
		for (int32 a = 0; a < FileList.Num(); a++) {
			UE_LOG(LogTemp, Error, TEXT("%s"), *FileList[a])
		}
		UObjectLibrary *ObjectLibrary = NewObject<UObjectLibrary>(UObjectLibrary::StaticClass());
		ObjectLibrary->UseWeakReferences(true);

		UE_LOG(LogTemp, Log, TEXT("Before load asset"));
		
		FString path = FString(TEXT("/Engine/") + PakAsset->Id);
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		TArray<FString> paths = TArray<FString>();
		paths.Add(path);

		AssetRegistry.ScanPathsSynchronous(paths, true);
		ObjectLibrary->LoadAssetDataFromPath(path);
		if (!ObjectLibrary->IsLibraryFullyLoaded()) {
			UE_LOG(LogTemp, Log, TEXT("Loading Object Library"));
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("Fully Loaded"));
		}
		ObjectLibrary->LoadAssetsFromAssetData();
		int32 count = ObjectLibrary->GetAssetDataCount();
		TArray<FAssetData> AssetDataList;
		ObjectLibrary->GetAssetDataList(AssetDataList);
		//return TArray<UStaticMesh*>();

		FStreamableManager* Streamable = new FStreamableManager();// UGameGlobals::Get().StreamableManager;
		UE_LOG(LogTemp, Log, TEXT("count:%d"), count);
		for (int32 x = 0; x < count; x++) {
			FAssetData AssetData = AssetDataList[x];
			FString Name = AssetData.ObjectPath.ToString();
			FString Class = AssetData.AssetClass.ToString();
			UE_LOG(LogTemp, Warning, TEXT("Name: %s"), *Name);
			UE_LOG(LogTemp, Warning, TEXT("Class: %s"), *Class);
			if (Class == FString("StaticMesh"))
			{
				UE_LOG(LogTemp, Warning, TEXT("Class is static mesh. Yielding: %s"), *Name);
                UStaticMesh* myMesh = LoadObject<UStaticMesh>(nullptr, *Name);
//                UStaticMesh* myMesh = Cast<UStaticMesh>(StaticLoadObject(UObject::StaticClass(), NULL, *Name));
//                if (!myMesh) {
//                    myMesh = LoadObject<UStaticMesh>(NULL, *Name, NULL, LOAD_None, NULL);
//                }
//                if (!myMesh) {
//                    TAssetPtr<UStaticMesh> BaseMesh;
//                    FStringAssetReference AssetRef = AssetData.ToSoftObjectPath();
//                    BaseMesh = Cast<UStaticMesh>(Streamable->LoadSynchronous(AssetRef, true));
//                    myMesh = BaseMesh.Get();
//                }
				result.Add(myMesh);
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadPak::MountPackage] Mounted to %s. Returning result..."), *MountPoint);
#if WITH_EDITOR
		PakPlatform->Unmount(*PackageFile);
		ObjectLibrary->ClearLoaded();
#endif
		return result;
	}
	return result;
}

const ANSICHAR* UAsyncTaskDownloadPak::GetKey() {
	return (const ANSICHAR*)"e3VqgSMhuaPw75fm0PdGZCN3ASwpVOk5Ij7iLf8VOEdqGL6aw05JeX0RHMgBvypd";
}
