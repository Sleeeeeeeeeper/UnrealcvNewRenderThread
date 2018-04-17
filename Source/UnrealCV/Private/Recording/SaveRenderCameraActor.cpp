// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealCVPrivate.h"
#include "SaveRenderCameraActor.h"
#include "Engine.h"
#include "TextureResource.h"

#include "HighResScreenshot.h"
#include "Paths.h"

#include <vector>
#include "RenderRequest.h"



ASaveRenderCameraActor::ASaveRenderCameraActor()
{
	CaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("NewSceneCaptureComponent2D"));
	CaptureComponent2D->SetupAttachment(RootComponent);
	CaptureComponent2DDeep = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("NewSceneCaptureComponent2DDeep"));
	CaptureComponent2DDeep->SetupAttachment(RootComponent);

	CaptureRenderTarget0 = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("CaptureRenderTarget0"));
	CaptureRenderTargetDeep = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("CaptureRenderTargetDeep"));


}

void ASaveRenderCameraActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	RenderResult.Init(nullptr, 3);

}

void ASaveRenderCameraActor::BeginPlay()
{
	//RenderResult.Init(nullptr, 3);
	//CaptureComponent2D->TextureTarget = RenderResult[0];
	//CaptureComponent2D2->TextureTarget = RenderResult[1];
	//SizeX = 720u;
	//SizeY = 512u;
	// Setup render target.
	const bool bInForceLinearGamma = true;
	CaptureRenderTarget0->InitCustomFormat(SizeX, SizeY, PF_B8G8R8A8, bInForceLinearGamma);
	CaptureRenderTargetDeep->InitCustomFormat(SizeX, SizeY, PF_B8G8R8A8, bInForceLinearGamma);

	CaptureComponent2D->TextureTarget = CaptureRenderTarget0;
	CaptureComponent2DDeep->TextureTarget = CaptureRenderTargetDeep;

	Super::BeginPlay();
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("ssssss"));
	//SaveCaptureToDisk(FilePath);
}

bool ASaveRenderCameraActor::ReadPixels(TArray<FColor> &BitMap, UTextureRenderTarget2D* CaptureRenderTargetIns) const
{
	//CaptureRenderTargetIns->UpdateResourceImmediate();
	//FTextureRenderTargetResource* RTResource = static_cast< FTextureRenderTargetResource*>(CaptureRenderTargetIns->Resource);
	FTextureRenderTargetResource* RTResource = CaptureRenderTargetIns->GameThread_GetRenderTargetResource();
	if (RTResource == nullptr)
	{
		return false;
	}
	FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
	ReadPixelFlags.SetLinearToGamma(true);
	return RTResource->ReadPixels(BitMap, ReadPixelFlags);

}

bool ASaveRenderCameraActor::SaveCaptureToDisk(const FString& FileName, RenderTargetType RenderT) const
{
	const FString FilePathAndName = FPaths::Combine(FPaths::ProjectSavedDir(), FileName);
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("CallSaveFunctionSuccess"));
	TArray<FColor> OutBMP;
	/*if (!ReadPixels(OutBMP, CaptureRenderTarget0)) {
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("NoPixels"));
	return false;
	}*/
	const FIntPoint DestSize(SizeX, SizeY);
	FString ResultPath;
	FHighResScreenshotConfig &HighResScreenshotConfig = GetHighResScreenshotConfig();
	if (RenderT == RenderTargetType::DefaultScene)
	{
		if (ReadPixels(OutBMP, CaptureRenderTarget0))
		{
			for (FColor &color : OutBMP) {
				color.A = 255;
			}
			return HighResScreenshotConfig.SaveImage(FilePathAndName, OutBMP, DestSize, &ResultPath);
		}
	}
	if (RenderT == RenderTargetType::Depth)
	{

		if (ReadPixels(OutBMP, CaptureRenderTargetDeep))
		{
			for (FColor &color : OutBMP) {
				color.A = 255;
			}
			return HighResScreenshotConfig.SaveImage(FilePathAndName, OutBMP, DestSize, &ResultPath);
		}
	}
	return false;
}

void ASaveRenderCameraActor::PrintHello()
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Hello"));
}

bool ASaveRenderCameraActor::RenderThreadSaveCaptureToDisk(const FString& FilePathT, const FString& FileName, RenderTargetType RenderT,int IWidth,int IHeight) const
{
	std::vector<std::shared_ptr<RenderRequest::RenderParams>> render_params;
	render_params.push_back(std::make_shared<RenderRequest::RenderParams>((RenderT == RenderTargetType::DefaultScene ? CaptureRenderTarget0 : CaptureRenderTargetDeep), false, true));
	std::vector<std::shared_ptr<RenderRequest::RenderResult>> render_results;


	RenderRequest render_request(false);
	render_request.getScreenshot(render_params.data(), render_results, render_params.size());

	
	const FIntPoint DestSize(SizeX, SizeY);
	FString FilePath = FilePathT;
	const FString FilePathAndName = FPaths::Combine(FilePath, FileName);
	FString ResultPath;
	FHighResScreenshotConfig &HighResScreenshotConfig = GetHighResScreenshotConfig();
	return HighResScreenshotConfig.SaveImage(FilePathAndName, render_results[0]->bmp, DestSize, &ResultPath);

	return false;
}
