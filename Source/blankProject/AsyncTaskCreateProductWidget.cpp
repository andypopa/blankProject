#include "AsyncTaskCreateProductWidget.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
//----------------------------------------------------------------------//
// UAsyncTaskCreateProductWidget
//----------------------------------------------------------------------//

UAsyncTaskCreateProductWidget::UAsyncTaskCreateProductWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		AddToRoot();
	}
}

UAsyncTaskCreateProductWidget* UAsyncTaskCreateProductWidget::CreateProductWidget(UPkg* pkg)
{
	UAsyncTaskCreateProductWidget* DownloadTask = NewObject<UAsyncTaskCreateProductWidget>();
	DownloadTask->Pkg = pkg;
	DownloadTask->Start(DownloadTask->Pkg->Thumb);
	return DownloadTask;
}

void UAsyncTaskCreateProductWidget::Start(FString URL)
{
	UAsyncTaskDownloadImage* DownloadTask = NewObject<UAsyncTaskDownloadImage>();
	DownloadTask->OnSuccess.AddDynamic(this, &UAsyncTaskCreateProductWidget::OnDownloadImageSuccess);
	//DownloadTask->OnFail.AddDynamic(this, &UAsyncTaskCreateProductWidget::OnDownloadImageSuccess);
	DownloadTask->Start(URL);
}

void UAsyncTaskCreateProductWidget::OnStatic(UTexture2DDynamic* Texture)
{
	UE_LOG(LogTemp, Error, TEXT("[UAsyncTaskCreateProductWidget::OnStatic] I'M SO STATIC %s"), *Texture->GetFullName());
}

void UAsyncTaskCreateProductWidget::OnDownloadImageSuccess(UTexture2DDynamic* Texture)
{
	this->Pkg->ThumbTexture = Texture;
	auto className = TEXT("/Game/Add_Product_Menu/Single_product_holder.Single_product_holder");
	UClass* MyBpClass = FindObject<UClass>(nullptr, className, true);
	if (MyBpClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UAsyncTaskCreateProductWidget::OnDownloadImageSuccess] Class not found %s"), *className);
	}
	//auto widget = CreateWidget<UUserWidget>(this, USingleProductHolder::StaticClass());
}

void UAsyncTaskCreateProductWidget::OnDownloadImageFail(UTexture2DDynamic* Texture)
{

}
//void UAsyncTaskCreateProductWidget::HandleStringRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
//{
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
//}
