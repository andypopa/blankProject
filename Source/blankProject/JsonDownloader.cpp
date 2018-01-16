#include "JsonDownloader.h"
#include "Runtime/PakFile/Public/IPlatformFilePak.h"
//#include "HitMeSingleton.h"
#include "blankProject.h"
#include "Json.h"

UJsonDownloader* UJsonDownloader::GetJsonDownloader(FString PackageName, FString URL, bool& IsValid)
{
	IsValid = false;

	UJsonDownloader *Object = NewObject<UJsonDownloader>();

	if (!Object) {
		UE_LOG(LogTemp, Error, TEXT("[UJsonDownloader::GetJsonDownloader] Could not be created"));
		return NULL;
	}
	if (!Object->IsValidLowLevel()) {
		UE_LOG(LogTemp, Error, TEXT("[UJsonDownloader::GetJsonDownloader] Created object is not valid"));
		return NULL;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UJsonDownloader::GetJsonDownloader] An instance is created successfully"));

	Object->OriginalPackageName = FString(PackageName);
	Object->OriginalURL = FString(URL);
	IsValid = true;
	return Object;
}
//
//void UJsonDownloader::UpdateCheckHttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
//{
//	UpdateRequest.Reset();
//
//	if (!Response.IsValid()) {
//		UE_LOG(LogTemp, Error, TEXT("[UJsonDownloader::UpdateCheckHttpRequestComplete] Could not connect to %s"), *RequestUrl);
//		OnUpdateCheckCompleted.Broadcast(OriginalPackageName, false);
//		return;
//	}
//
//	if (!bWasSuccessful) {
//		UE_LOG(LogTemp, Error, TEXT("[UJsonDownloader::UpdateCheckHttpRequestComplete] Could not connect to %s"), *RequestUrl);
//		OnUpdateCheckCompleted.Broadcast(OriginalPackageName, false);
//		return;
//	}
//
//	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
//	FFileStatData statData = PlatformFile.GetStatData(*dataFile);
//
//	bool isSizeDifferent = false;
//	bool isModDateDifferent = false;
//
//	int64 fileSize = 0;
//	int64 modDate = 0;
//
//	{
//		int32 StatusCode = Response->GetResponseCode();
//		if (StatusCode / 100 != 2)
//		{
//			UE_LOG(LogTemp, Error, TEXT("[UJsonDownloader::UpdateCheckHttpRequestComplete] %s HTTP response %d, for %s"), *OriginalPackageName, StatusCode, *RequestUrl);
//			OnUpdateCheckCompleted.Broadcast(OriginalPackageName, false);
//			return;
//		}
//
//		TArray<FString> headers = Response->GetAllHeaders();
//		for (FString h : headers) {
//			UE_LOG(LogTemp, Warning, TEXT("[UJsonDownloader::UpdateCheckHttpRequestComplete] %s Header: %s"), *OriginalPackageName, *h);
//		}
//
//		for (FString h : headers) {
//			if (h.Contains("x-file-size", ESearchCase::IgnoreCase) || h.Contains("Content-Length", ESearchCase::IgnoreCase)) {
//				FString left;
//				FString right;
//				h.Split(":", &left, &right, ESearchCase::IgnoreCase, ESearchDir::FromStart);
//				if (right.Len())
//				{
//					fileSize = FCString::Atoi(*right);
//				}
//			}
//
//			if (h.Contains("x-file-mod", ESearchCase::IgnoreCase)) {
//				FString left;
//				FString right;
//				h.Split(":", &left, &right, ESearchCase::IgnoreCase, ESearchDir::FromStart);
//				if (right.Len())
//				{
//					modDate = FCString::Atoi(*right);
//				}
//			}
//
//			if (h.Contains("Last-Modified", ESearchCase::IgnoreCase)) {
//				FString left;
//				FString right;
//				h.Split(":", &left, &right, ESearchCase::IgnoreCase, ESearchDir::FromStart);
//				if (right.Len())
//				{
//					right = right.Trim().TrimTrailing();
//					FDateTime date;
//					FDateTime::ParseHttpDate(right, date);
//					modDate = date.ToUnixTimestamp();
//				}
//			}
//		}
//
//		Request.Reset();
//	}
//
//	UE_LOG(LogTemp, Warning, TEXT("[UJsonDownloader::UpdateCheckHttpRequestComplete] %s - REMOTE: File size %i Mod date %i"), *OriginalPackageName, fileSize, modDate);
//	UE_LOG(LogTemp, Warning, TEXT("[UJsonDownloader::UpdateCheckHttpRequestComplete] %s - LOCAL:  File size %i Mod date %i"), *OriginalPackageName, statData.FileSize, statData.ModificationTime.ToUnixTimestamp());
//
//	isSizeDifferent = fileSize > 0 && statData.FileSize != fileSize;
//	isModDateDifferent = modDate > 0 && modDate > statData.ModificationTime.ToUnixTimestamp();
//	OnUpdateCheckCompleted.Broadcast(OriginalPackageName, isSizeDifferent || isModDateDifferent);
//}

void UJsonDownloader::DownloadPackage()
{
	//CreatePackageFolder();

	RequestSize = -1;
	RequestUrl = OriginalURL;
	UE_LOG(LogTemp, Warning, TEXT("[UJsonDownloader::DownloadPackage] %s Requesting headers for %s"), *OriginalURL, *OriginalURL);

	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> HttpRequest = Http->CreateRequest();
	DownloadRequest = HttpRequest;

	HttpRequest->SetVerb(TEXT("HEAD"));
	HttpRequest->SetURL(RequestUrl);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UJsonDownloader::HttpRequestComplete);
	if (!HttpRequest->ProcessRequest())
	{
		OnJsonDownloaderror.Broadcast(OriginalPackageName);
	}
}

void UJsonDownloader::HttpRequestProgress(FHttpRequestPtr Request, int32 bytesSent, int32 bytesReceived)
{
	if (RequestSize <= 0) return;

	float percent = (float)bytesReceived / (float)RequestSize;
	OnPackageDownloadProgress.Broadcast(OriginalPackageName, (int32)(percent * 100));
}

void UJsonDownloader::HttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	DownloadRequest.Reset();

	if (!Response.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("[UJsonDownloader::HttpRequestComplete] Could not connect to %s"), *RequestUrl);
		OnJsonDownloaderror.Broadcast(OriginalPackageName);
		return;
	}

	if (!bWasSuccessful) {
		UE_LOG(LogTemp, Error, TEXT("[UJsonDownloader::HttpRequestComplete] Could not connect to %s"), *RequestUrl);
		OnJsonDownloaderror.Broadcast(OriginalPackageName);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UJsonDownloader::HttpRequestComplete] %s Starting download of %s"), *OriginalPackageName, *RequestUrl);

	// Finding size of the requested file
	{
		int32 StatusCode = Response->GetResponseCode();
		if (StatusCode / 100 != 2)
		{
			UE_LOG(LogTemp, Error, TEXT("[UJsonDownloader::HttpRequestComplete] %s HTTP response %d, for %s"), *OriginalPackageName, StatusCode, *RequestUrl);
			OnJsonDownloaderror.Broadcast(OriginalPackageName);
			return;
		}

		TArray<FString> headers = Response->GetAllHeaders();
		for (FString h : headers) {
			UE_LOG(LogTemp, Warning, TEXT("[UJsonDownloader::HttpRequestComplete] %s Header: %s"), *OriginalPackageName, *h);
		}

		for (FString h : headers) {
			if (h.Contains("x-file-size", ESearchCase::IgnoreCase) || h.Contains("Content-Length", ESearchCase::IgnoreCase)) {
				FString left;
				FString right;
				h.Split(":", &left, &right, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				if (right.Len())
				{
					RequestSize = FCString::Atoi(*right);
				}

				break;
			}
		}

		Request.Reset();
	}

	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> HttpRequest = Http->CreateRequest();
	DownloadRequest = HttpRequest;

	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(RequestUrl);
	HttpRequest->OnRequestProgress().BindUObject(this, &UJsonDownloader::HttpRequestProgress);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UJsonDownloader::HttpDownloadComplete);
	if (!HttpRequest->ProcessRequest())
	{
		OnJsonDownloaderror.Broadcast(OriginalPackageName);
	}
}

void UJsonDownloader::HttpDownloadComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	DownloadRequest.Reset();

	if (!Response.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("[UJsonDownloader::HttpDownloadComplete] Could not connect to %s"), *RequestUrl);
		OnJsonDownloaderror.Broadcast(OriginalPackageName);
		return;
	}

	if (!bWasSuccessful) {
		UE_LOG(LogTemp, Error, TEXT("[UJsonDownloader::HttpDownloadComplete] Could not connect to %s"), *RequestUrl);
		OnJsonDownloaderror.Broadcast(OriginalPackageName);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UJsonDownloader::HttpDownloadComplete] Download completed for %s from %s"), *OriginalPackageName, *RequestUrl);

	int32 StatusCode = Response->GetResponseCode();
	if (StatusCode / 100 != 2)
	{
		UE_LOG(LogTemp, Error, TEXT("[UJsonDownloader::HttpDownloadComplete] %s HTTP response %d, for %s"), *OriginalPackageName, StatusCode, *RequestUrl);
		OnJsonDownloaderror.Broadcast(OriginalPackageName);
		return;
	}

	TArray<FString> headers = Response->GetAllHeaders();
	for (FString h : headers) {
		UE_LOG(LogTemp, Warning, TEXT("UExpoSocket::HttpDownloadComplete] %s Header: %s"), *OriginalPackageName, *h);
	}

	const TArray<uint8>& Content = Response->GetContent();
	FString szJson = Response->GetContentAsString();

	TSharedPtr<FJsonObject> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(szJson);
	bool result = FJsonSerializer::Deserialize(JsonReader, JsonParsed);
	if (!result)
	{
		UE_LOG(LogTemp, Error, TEXT("FJsonSerializer: FAILED TO DESERIALIZE"));
	}
	auto data = JsonParsed->GetArrayField(FString("data"));
	TArray<FString> PackageNames = TArray<FString>();
	for (int i = 0; i < data.Num(); ++i)
	{
		TSharedPtr<FJsonObject> item = data[i]->AsObject();
		FString itemSz = item->GetStringField(FString("name"));
		FString pakSz = item->GetStringField(FString("pak"));
		FString thumbSz = item->GetStringField(FString("thumb"));
		OnPackageDownloadCompleted.Broadcast(itemSz, pakSz, thumbSz);

		PackageNames.Add(itemSz);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *itemSz);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *pakSz);
	}
}


void UJsonDownloader::CancelDownload()
{
	UE_LOG(LogTemp, Error, TEXT("[UJsonDownloader::CancelDownload] Cancelling request for %s"), *RequestUrl);

	if (UpdateRequest.IsValid()) {
		if (UpdateRequest->OnProcessRequestComplete().IsBound())
			UpdateRequest->OnProcessRequestComplete().Unbind();

		UpdateRequest->CancelRequest();
		UpdateRequest.Reset();
	}

	if (DownloadRequest.IsValid()) {
		if (DownloadRequest->OnProcessRequestComplete().IsBound())
			DownloadRequest->OnProcessRequestComplete().Unbind();

		DownloadRequest->CancelRequest();
		DownloadRequest.Reset();
	}
}