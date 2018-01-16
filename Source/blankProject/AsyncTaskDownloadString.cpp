#include "AsyncTaskDownloadString.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"


//----------------------------------------------------------------------//
// UAsyncTaskDownloadString
//----------------------------------------------------------------------//

UAsyncTaskDownloadString::UAsyncTaskDownloadString(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		AddToRoot();
	}
}

UAsyncTaskDownloadString* UAsyncTaskDownloadString::DownloadString(FString URL)
{
	UAsyncTaskDownloadString* DownloadTask = NewObject<UAsyncTaskDownloadString>();
	DownloadTask->Start(URL);

	return DownloadTask;
}

void UAsyncTaskDownloadString::Start(FString URL)
{
#if !UE_SERVER
	// Create the Http request and add to pending request list
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UAsyncTaskDownloadString::HandleStringRequest);
	HttpRequest->SetURL(URL);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->ProcessRequest();
#else
	// On the server we don't execute fail or success we just don't fire the request.
	RemoveFromRoot();
#endif
}

void UAsyncTaskDownloadString::HandleStringRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
#if !UE_SERVER
	RemoveFromRoot();

	if (bSucceeded && HttpResponse.IsValid() && HttpResponse->GetContentLength() > 0)
	{
		OnSuccess.Broadcast(HttpResponse->GetContentAsString());
			return;
	}

	OnFail.Broadcast(FString("Failed"));
#endif
}
