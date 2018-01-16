#include "AsyncTaskDownloadSig.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "PlatformFilemanager.h"
#include "Runtime/PakFile/Public/IPlatformFilePak.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#include "CoreDelegates.h"
#include "AssetRegistryModule.h"
#include "Engine/StreamableManager.h"
#include "ConstructorHelpers.h"
#include "PakAsset.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
//----------------------------------------------------------------------//
// UAsyncTaskDownloadSig
//----------------------------------------------------------------------//

UAsyncTaskDownloadSig::UAsyncTaskDownloadSig(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		AddToRoot();
	}
}

UAsyncTaskDownloadSig* UAsyncTaskDownloadSig::DownloadSig(UPakAsset* PakAsset)
{
	UAsyncTaskDownloadSig* DownloadTask = NewObject<UAsyncTaskDownloadSig>();
	DownloadTask->PakAsset = PakAsset;
	DownloadTask->Start();
	return DownloadTask;
}

bool UAsyncTaskDownloadSig::CheckIfSigExists(UPakAsset* PakAsset)
{
	FString Filename = PackageFolder() + "/" + PakAsset->Id + ".sig";
	if (FPaths::FileExists(Filename))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadSig::CheckIfSigExists] %s SIG EXISTS"), *PakAsset->Name);
		return true;
	}
	return false;
}

void UAsyncTaskDownloadSig::Start()
{
	FString Filename = PackageFolder() + "/" + this->PakAsset->Id + ".sig";
	if (FPaths::FileExists(Filename))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadSig::DownloadPackage] %s PAK ALREADY EXISTS. Broadcasting OnSuccess."), *this->PakAsset->Name);
		OnSuccess.Broadcast(this->PakAsset->Name);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UAsyncTaskDownloadSig::DownloadPackage] %s Requesting headers for %s"), *this->PakAsset->Name, *this->PakAsset->Sig);

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(this->PakAsset->Sig);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UAsyncTaskDownloadSig::HandleSigRequest);
	if (!HttpRequest->ProcessRequest())
	{
		OnFail.Broadcast(this->PakAsset->Name);
	}
}

void UAsyncTaskDownloadSig::HandleSigRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
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

	FString Filename = PackageFolder() + "/" + this->PakAsset->Id + ".sig";
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

FString UAsyncTaskDownloadSig::PackageFolder()
{
	FString FileDir = FPaths::ProjectPersistentDownloadDir() + "/DownPaks/";
	FPaths::NormalizeDirectoryName(FileDir);
	return FString(FPaths::ConvertRelativePathToFull(FileDir));
}
