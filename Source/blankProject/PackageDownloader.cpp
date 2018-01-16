#include "PackageDownloader.h"
#include "blankProject.h"
#include "IPlatformFilePak.h"
#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
#include "FileManager.h"
#include "PlatformFilemanager.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#include "CoreDelegates.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Engine/StaticMesh.h"

class FPakFileVisitor : public IPlatformFile::FDirectoryVisitor
{
public:
	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
	{
		if (!bIsDirectory)
		{
			Files.Add(FilenameOrDirectory);
		}
		return true;
	}

	TArray<FString> Files;
};

UPackageDownloader* UPackageDownloader::GetPackageDownloader(FString PackageName, FString URL, bool& IsValid)
{
	IsValid = false;

	UPackageDownloader *Object = NewObject<UPackageDownloader>();

	if (!Object) {
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::GetPackageDownloader] Could not be created"));
		return NULL;
	}
	if (!Object->IsValidLowLevel()) {
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::GetPackageDownloader] Created object is not valid"));
		return NULL;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::GetPackageDownloader] An instance is created successfully"));

	Object->OriginalPackageName = FString(PackageName);
	Object->OriginalURL = FString(URL);
	//Object->AssetRegistryModule = 
	//Object->AssetRegistryModule.Get().OnAssetAdded().AddRaw(Object, &UPackageDownloader::OnAssetAdded);
	IsValid = true;
	//.AddRaw(Object, &UPackageDownloader::OnAssetAdded);//.
	//Object->AssetRegistryModule.Get().OnAssetAdded().AddSP(Object, &UPackageDownloader::OnAssetAdded);
	return Object;
}

void UPackageDownloader::OnAssetAdded(const FAssetData& AssetData)
{
	// An asset was discovered by the asset registry.
	// This means it was either just created or recently found on disk.
	// Make sure code in this function is fast or it will slow down the gathering process.
	FString klass = AssetData.AssetClass.ToString();
	//FString type = klass->GetName();
	UE_LOG(LogTemp, Warning, TEXT("Klass: %s"), *klass);

}

FString UPackageDownloader::PackageFolder()
{
	FString FileDir = FPaths::ProjectPersistentDownloadDir() + "/DownPaks/";
	FPaths::NormalizeDirectoryName(FileDir);
	return FString(FPaths::ConvertRelativePathToFull(FileDir));
}

bool UPackageDownloader::CreatePackageFolder()
{
	FDirPackageRecursiveDownloader RFolder = FDirPackageRecursiveDownloader::CreateLambda([=](FString Folder)
	{
		const int32 MAX_LOOP_ITR = 3000;
		FPaths::NormalizeDirectoryName(Folder);
		Folder += "/";

		FString Base;
		FString Left;
		FString Remaining;

		Folder.Split(TEXT("/"), &Base, &Remaining);
		Base += "/";

		int32 LoopItr = 0;
		while (Remaining != "" && LoopItr < MAX_LOOP_ITR)
		{
			Remaining.Split(TEXT("/"), &Left, &Remaining);
			Base += Left + FString("/");
			FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*Base);

			UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::CreatePackageFolder] Creating %s"), *Base);
			LoopItr++;
		}
	});

	FString FileDir = PackageFolder();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*FileDir)) {
		RFolder.Execute(FileDir);

		if (!PlatformFile.DirectoryExists(*FileDir)) {
			UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::CreatePackageFolder] Cannot create folder %s"), *FileDir);
			return false;
		}

		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::CreatePackageFolder] Created folder %s"), *FileDir);
	}

	return true;
}

TArray<FString> UPackageDownloader::DownloadedPackagesList()
{
	FString folder = PackageFolder() + "/*.*";
	IFileManager& FileManager = IFileManager::Get();

	TArray<FString>files;
	FileManager.FindFiles(files, *folder, true, false);
	for (int i = 0; i < files.Num(); i++) {
		FString str = files[i];
		files[i] = str.Replace(TEXT(".pak"), TEXT(""), ESearchCase::IgnoreCase);
	}
	return files;
}

void UPackageDownloader::IsPackageDownloaded(FString PackageName, bool& isDownloaded)
{
	if (!CreatePackageFolder())
	{
		isDownloaded = false;
		return;
	}

	FString dataFile = PackageFolder() + "/" + PackageName + ".pak";
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	isDownloaded = PlatformFile.FileExists(*dataFile);
}

void UPackageDownloader::DeletePackageFile(FString PackageName, bool &isDeleted)
{
	FString dataFile = PackageFolder() + "/" + PackageName + ".pak";
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.FileExists(*dataFile))
	{
		isDeleted = true;
		return;
	}

	isDeleted = PlatformFile.DeleteFile(*dataFile);
}

void UPackageDownloader::IsPackageMounted(FString PackageName, bool& isMounted)
{
	//UHitMeSingleton *Object = Cast<UHitMeSingleton>(GEngine->GameSingleton);

	//if (!(Object && Object->IsValidLowLevel())) {
	//	isMounted = false;
	//	return;
	//}

	//isMounted = Object->mountedPackages.Contains(PackageName);
}

bool UPackageDownloader::HotfixPakFile(FString Path)
{
	// Patching the editor this way seems like a bad idea
	bool bShouldHotfix = IsRunningGame() || IsRunningDedicatedServer() || IsRunningClientOnly();
	if (!bShouldHotfix)
	{
		UE_LOG(LogTemp, Error, TEXT("Hotfixing skipped when not running game/server"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Hotfixing RUNNING"));
	}

	auto Filename = FPaths::GetBaseFilename(Path);
	if (!FCoreDelegates::OnMountPak.IsBound())
	{
		UE_LOG(LogTemp, Error, TEXT("PAK file (%s) could not be mounted because OnMountPak is not bound"), *Filename);
		return false;
	}
	FString FileDir = FPaths::ProjectPersistentDownloadDir() + "/DownPaks/";
	FString PakLocation = *Path; //FString::Printf(TEXT("%s/%s"), *FileDir, *Path);
	FPakFileVisitor Visitor;
	if (FCoreDelegates::OnMountPak.Execute(PakLocation, 0, &Visitor))
	{
		//whatever

		//MountedPakFiles.Add(*path);
		UE_LOG(LogTemp, Log, TEXT("Hotfix mounted PAK file (%s)"), *Filename);
		/*int32 NumInisReloaded = 0;
		const double StartTime = FPlatformTime::Seconds();
		TArray<FString> IniList;*/
		// Iterate through the pak file's contents for INI and asset reloading
		for (auto& InternalPakFileName : Visitor.Files)
		{
			/*if (InternalPakFileName.EndsWith(TEXT(".ini")))
			{
				IniList.Add(InternalPakFileName);
			}
			else*/
			UE_LOG(LogTemp, Log, TEXT("Found in PAK: (%s)"), *InternalPakFileName);
		}
		// Sort the INIs so they are processed consistently
//		IniList.Sort<FHotfixFileSortPredicate>(FHotfixFileSortPredicate(PlatformPrefix, ServerPrefix, DefaultPrefix));
//		// Now process the INIs in sorted order
//		for (auto& IniName : IniList)
//		{
//			HotfixPakIniFile(IniName);
//			NumInisReloaded++;
//		}
//		UE_LOG(LogTemp, Log, TEXT("Processing pak file (%s) took %f seconds and resulted in (%d) INIs being reloaded"),
//			*FileHeader.FileName, FPlatformTime::Seconds() - StartTime, NumInisReloaded);
//#if !UE_BUILD_SHIPPING
		//if (bLogMountedPakContents)
		//{
			UE_LOG(LogTemp, Log, TEXT("Files in pak file (%s):"), *Filename);
			for (auto& FileName : Visitor.Files)
			{
				UE_LOG(LogTemp, Log, TEXT("\t\t%s"), *FileName);
			}
		//}
//#endif
		return true;
	}
	return false;
}
//
//bool UOnlineHotfixManager::IsMapLoaded(const FString& MapName)
//{
//	FString MapPackageName(MapName.Left(MapName.Len() - 5));
//	MapPackageName = MapPackageName.Replace(*GameContentPath, TEXT("/Game"));
//	// If this map's UPackage exists, it is currently loaded
//	UPackage* MapPackage = FindObject<UPackage>(ANY_PACKAGE, *MapPackageName, true);
//	return MapPackage != nullptr;
//}


TArray<UStaticMesh*> UPackageDownloader::MountPackage(FString PackageName, FString MountFolder, bool& isMounted)
{
	UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::MountPackage] Mounting package %s"), *PackageName);

	FString PackageFile = PackageFolder() + "/" + PackageName + ".pak";
	isMounted = false;
	bool bSuccessfulInitialization = false;

	IPlatformFile* LocalPlatformFile = nullptr;// = &FPlatformFileManager::Get().GetPlatformFile();
	//IPlatformFile LPF = *(&FPlatformFileManager::Get().GetPlatformFile());
	if (LocalPlatformFile != nullptr)
	{
		IPlatformFile* PakPlatformFile = FPlatformFileManager::Get().GetPlatformFile(TEXT("PakFile"));
		if (PakPlatformFile != nullptr) bSuccessfulInitialization = true;
	}
	TArray<UStaticMesh*> result = TArray<UStaticMesh*>();

	if (bSuccessfulInitialization)
	{
		const TCHAR* cmdLine = TEXT("");
		FPakPlatformFile* PakPlatform = new FPakPlatformFile();
		//IPlatformFile* InnerPlatform = LocalPlatformFile;
		IPlatformFile* InnerPlatform = FPlatformFileManager::Get().GetPlatformFile(TEXT("PakFile"));
		if (InnerPlatform != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::MountPackage] MOUNTED WITH NEW METHOD"), *PackageName);

		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::MountPackage] FAILED MOUNT. IS NULLPTR."), *PackageName);
			return result;
		}
		PakPlatform->Initialize(InnerPlatform, cmdLine);
		FPlatformFileManager::Get().SetPlatformFile(*PakPlatform);
		const TCHAR* PkgFile = *PackageFile;
		FPakFile PakFile(InnerPlatform, *PackageFile, true);
		if (!PakFile.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::MountPackage] Invalid pak file %s"), *PackageName);
			return TArray<UStaticMesh*>();
		}

		FString MountPoint = FPaths::EngineContentDir() + PackageName;
		//FString MountPoint = FPaths::EngineContentDir() + MountFolder + "/" + PackageName;
		//FString MountPoint = FPaths::GameContentDir() + "FTP";
		UE_LOG(LogTemp, Warning, TEXT("Mount point: %s"), *MountPoint);
		PakFile.SetMountPoint(*MountPoint);

		const int32 PakOrder = 0;

		//#if WITH_EDITOR
		//		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::MountPackage] Not mounting %s/%s in editor"), *MountPoint, *PackageFile);
		//#else
		if (!PakPlatform->Mount(*PackageFile, PakOrder, *MountPoint))
		{
			UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::MountPackage] Failed to mount package %s"), *MountPoint);
			return result;
		}
		//#endif

		isMounted = true;
		//UHitMeSingleton *Object = Cast<UHitMeSingleton>(GEngine->GameSingleton);
		//if (Object && Object->IsValidLowLevel()) {
		//	Object->mountedPackages.Add(PackageName);
		//}

		// Package location control
		TArray<FString> FileList;
		PakFile.FindFilesAtPath(FileList, *PakFile.GetMountPoint(), true, false, true);
		for (int32 a = 0; a < FileList.Num(); a++) {
			UE_LOG(LogTemp, Error, TEXT("%s"), *FileList[a])
		}
		UObjectLibrary *ObjectLibrary = NewObject<UObjectLibrary>(UObjectLibrary::StaticClass());
		ObjectLibrary->UseWeakReferences(false);

		UE_LOG(LogTemp, Log, TEXT("Before load asset"));
		FString path = FString(TEXT("/Engine/") + PackageName);
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
				result.Add(myMesh);
			}
		}
		//for (int32 b = 0; b < FileList.Num(); b++) {
		//	FString file = FileList[b];
		//	UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::MountPackage] Content %s: %s"), *PackageName, *file);
		//	//FStreamableManager AssetLoader;
		//	//FStringAssetReference AssetRef(*file);
		//	//auto mMeshAsset = Cast< UStaticMesh>(AssetLoader.SynchronousLoad(AssetRef));
		//	FString EnginePath = FString(TEXT("/Engine/Content/" + PackageName + "/" + PackageName + "/"));
		//	FString left;
		//	FString right;
		//	file.Split("/", &left, &right, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		//	right.Split(".", &left, &right, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

		//	FString ShortAssetName = left + "." + left;
		//	FString AssetPath = EnginePath + ShortAssetName;
		//	UE_LOG(LogTemp, Warning, TEXT("Dis da AssetPath: %s"), *AssetPath);
		//	//UStaticMesh* myMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, *AssetPath));

		//	//FString name = *file;
		//	//FPaths::MakeStandardFilename(name);
		//	UStaticMesh* myMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, NULL, *AssetPath));
		//	result.Add(myMesh);
		//	//UE_LOG(LogTemp, Error, TEXT("DIS HERE DA MESH DAWG %s"), *(myMesh->GetFullName()));
		//	UE_LOG(LogTemp, Error, TEXT("WATCH YOSELF. RETURNIN DA MESH"));
		//	//return myMesh;
		//}
		UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::MountPackage] Mounted to %s. Returning result..."), *MountPoint);


		//FPlatformFileManager::Get().SetPlatformFile(LPF);
		return result;

		//mMesh->SetStaticMesh(mMeshAsset.Get());
	}
	return result;
}

void UPackageDownloader::CheckIfPackageHasUpdate()
{
	UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::DoesPackageHaveUpdate] Checking for update %s"), *OriginalPackageName);

	FString dataFile = PackageFolder() + "/" + OriginalPackageName + ".pak";
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.FileExists(*dataFile))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::DoesPackageHaveUpdate] Package not downloaded yet %s"), *OriginalPackageName);
		OnUpdateCheckCompleted.Broadcast(OriginalPackageName, true);
		return;
	}

	RequestSize = -1;
	RequestUrl = OriginalURL + "/" + OriginalPackageName + ".pak";
	UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::DoesPackageHaveUpdate] Requesting headers for %s from %s"), *OriginalPackageName, *RequestUrl);

	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> HttpRequest = Http->CreateRequest();
	UpdateRequest = HttpRequest;

	HttpRequest->SetVerb(TEXT("HEAD"));
	HttpRequest->SetURL(RequestUrl);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UPackageDownloader::UpdateCheckHttpRequestComplete);
	if (!HttpRequest->ProcessRequest())
	{
		OnUpdateCheckCompleted.Broadcast(OriginalPackageName, false);
	}
}

void UPackageDownloader::UpdateCheckHttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	UpdateRequest.Reset();

	if (!Response.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::UpdateCheckHttpRequestComplete] Could not connect to %s"), *RequestUrl);
		OnUpdateCheckCompleted.Broadcast(OriginalPackageName, false);
		return;
	}

	if (!bWasSuccessful) {
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::UpdateCheckHttpRequestComplete] Could not connect to %s"), *RequestUrl);
		OnUpdateCheckCompleted.Broadcast(OriginalPackageName, false);
		return;
	}

	FString dataFile = PackageFolder() + "/" + OriginalPackageName + ".pak";
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FFileStatData statData = PlatformFile.GetStatData(*dataFile);

	bool isSizeDifferent = false;
	bool isModDateDifferent = false;

	int64 fileSize = 0;
	int64 modDate = 0;

	{
		int32 StatusCode = Response->GetResponseCode();
		if (StatusCode / 100 != 2)
		{
			UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::UpdateCheckHttpRequestComplete] %s HTTP response %d, for %s"), *OriginalPackageName, StatusCode, *RequestUrl);
			OnUpdateCheckCompleted.Broadcast(OriginalPackageName, false);
			return;
		}

		TArray<FString> headers = Response->GetAllHeaders();
		for (FString h : headers) {
			UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::UpdateCheckHttpRequestComplete] %s Header: %s"), *OriginalPackageName, *h);
		}

		for (FString h : headers) {
			if (h.Contains("x-file-size", ESearchCase::IgnoreCase) || h.Contains("Content-Length", ESearchCase::IgnoreCase)) {
				FString left;
				FString right;
				h.Split(":", &left, &right, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				if (right.Len())
				{
					fileSize = FCString::Atoi(*right);
				}
			}

			if (h.Contains("x-file-mod", ESearchCase::IgnoreCase)) {
				FString left;
				FString right;
				h.Split(":", &left, &right, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				if (right.Len())
				{
					modDate = FCString::Atoi(*right);
				}
			}

			if (h.Contains("Last-Modified", ESearchCase::IgnoreCase)) {
				FString left;
				FString right;
				h.Split(":", &left, &right, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				if (right.Len())
				{
					right = right.TrimStartAndEnd();
					FDateTime date;
					FDateTime::ParseHttpDate(right, date);
					modDate = date.ToUnixTimestamp();
				}
			}
		}

		Request.Reset();
	}

	UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::UpdateCheckHttpRequestComplete] %s - REMOTE: File size %i Mod date %i"), *OriginalPackageName, fileSize, modDate);
	UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::UpdateCheckHttpRequestComplete] %s - LOCAL:  File size %i Mod date %i"), *OriginalPackageName, statData.FileSize, statData.ModificationTime.ToUnixTimestamp());

	isSizeDifferent = fileSize > 0 && statData.FileSize != fileSize;
	isModDateDifferent = modDate > 0 && modDate > statData.ModificationTime.ToUnixTimestamp();
	OnUpdateCheckCompleted.Broadcast(OriginalPackageName, isSizeDifferent || isModDateDifferent);
}

void UPackageDownloader::DownloadPackage()
{
	CreatePackageFolder();

	RequestSize = -1;
	RequestUrl = OriginalURL;
	UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::DownloadPackage] %s Requesting headers for %s"), *OriginalPackageName, *RequestUrl);

	//if .pak already exists
	FString Filename = PackageFolder() + "/" + OriginalPackageName + ".pak";
	if (FPaths::FileExists(Filename))
	{
		OnPackageDownloadCompleted.Broadcast(OriginalPackageName);
		return;
	}

	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> HttpRequest = Http->CreateRequest();
	DownloadRequest = HttpRequest;

	HttpRequest->SetVerb(TEXT("HEAD"));
	HttpRequest->SetURL(RequestUrl);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UPackageDownloader::HttpRequestComplete);
	if (!HttpRequest->ProcessRequest())
	{
		OnPackageDownloadError.Broadcast(OriginalPackageName);
	}
}

void UPackageDownloader::HttpRequestProgress(FHttpRequestPtr Request, int32 bytesSent, int32 bytesReceived)
{
	if (RequestSize <= 0) return;

	float percent = (float)bytesReceived / (float)RequestSize;
	OnPackageDownloadProgress.Broadcast(OriginalPackageName, (int32)(percent * 100));
}

void UPackageDownloader::HttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	DownloadRequest.Reset();

	if (!Response.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::HttpRequestComplete] Could not connect to %s"), *RequestUrl);
		OnPackageDownloadError.Broadcast(OriginalPackageName);
		return;
	}

	if (!bWasSuccessful) {
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::HttpRequestComplete] Could not connect to %s"), *RequestUrl);
		OnPackageDownloadError.Broadcast(OriginalPackageName);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::HttpRequestComplete] %s Starting download of %s"), *OriginalPackageName, *RequestUrl);

	// Finding size of the requested file
	{
		int32 StatusCode = Response->GetResponseCode();
		if (StatusCode / 100 != 2)
		{
			UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::HttpRequestComplete] %s HTTP response %d, for %s"), *OriginalPackageName, StatusCode, *RequestUrl);
			OnPackageDownloadError.Broadcast(OriginalPackageName);
			return;
		}

		TArray<FString> headers = Response->GetAllHeaders();
		for (FString h : headers) {
			UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::HttpRequestComplete] %s Header: %s"), *OriginalPackageName, *h);
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
	HttpRequest->OnRequestProgress().BindUObject(this, &UPackageDownloader::HttpRequestProgress);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UPackageDownloader::HttpDownloadComplete);
	if (!HttpRequest->ProcessRequest())
	{
		OnPackageDownloadError.Broadcast(OriginalPackageName);
	}
}

void UPackageDownloader::HttpDownloadComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	DownloadRequest.Reset();

	if (!Response.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::HttpDownloadComplete] Could not connect to %s"), *RequestUrl);
		OnPackageDownloadError.Broadcast(OriginalPackageName);
		return;
	}

	if (!bWasSuccessful) {
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::HttpDownloadComplete] Could not connect to %s"), *RequestUrl);
		OnPackageDownloadError.Broadcast(OriginalPackageName);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UPackageDownloader::HttpDownloadComplete] Download completed for %s from %s"), *OriginalPackageName, *RequestUrl);

	int32 StatusCode = Response->GetResponseCode();
	if (StatusCode / 100 != 2)
	{
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::HttpDownloadComplete] %s HTTP response %d, for %s"), *OriginalPackageName, StatusCode, *RequestUrl);
		OnPackageDownloadError.Broadcast(OriginalPackageName);
		return;
	}

	TArray<FString> headers = Response->GetAllHeaders();
	for (FString h : headers) {
		UE_LOG(LogTemp, Warning, TEXT("UExpoSocket::HttpDownloadComplete] %s Header: %s"), *OriginalPackageName, *h);
	}

	const TArray<uint8>& Content = Response->GetContent();

	FString Filename = PackageFolder() + "/" + OriginalPackageName + ".pak";
	UE_LOG(LogTemp, Error, TEXT("sunt o conditie bulangie"));
	if (FFileHelper::SaveArrayToFile(Content, *Filename))
	{
		UE_LOG(LogTemp, Error, TEXT("sunt o fucntie bulangie"));
		OnPackageDownloadCompleted.Broadcast(OriginalPackageName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::HttpDownloadComplete] %s Could not write %s to disk "), *OriginalPackageName, *Filename);
		OnPackageDownloadError.Broadcast(OriginalPackageName);
	}
}

void UPackageDownloader::CancelDownload()
{
	UE_LOG(LogTemp, Error, TEXT("[UPackageDownloader::CancelDownload] Cancelling request for %s"), *RequestUrl);

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
