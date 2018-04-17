// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "SaveRenderCameraActor.generated.h"


UENUM(BlueprintType)
enum class RenderTargetType : uint8
{
	DefaultScene UMETA(DisplayName = "Default"),
	Depth UMETA(DisplayName = "Depth")
};


/**
 * 
 */
UCLASS()
class ASaveRenderCameraActor : public ACameraActor
{
	GENERATED_BODY()
public:
	ASaveRenderCameraActor();


protected:
	// Called when the game starts or when spawned
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;




private:

	UPROPERTY(EditAnywhere)
		USceneCaptureComponent2D * CaptureComponent2D;
	UPROPERTY(EditAnywhere)
		USceneCaptureComponent2D * CaptureComponent2DDeep;
	UPROPERTY(EditAnywhere)
		TArray<UTextureRenderTarget2D*> RenderResult;


public:
	UPROPERTY(Transient)
		UTextureRenderTarget2D* CaptureRenderTarget0;

	UPROPERTY(Transient)
		UTextureRenderTarget2D* CaptureRenderTargetDeep;


	UPROPERTY(Category = "Scene Capture", EditAnywhere)
		uint32 SizeX = 720u;
	UPROPERTY(Category = "Scene Capture", EditAnywhere)
		uint32 SizeY = 512u;


public:
	UFUNCTION(BlueprintCallable)
		bool ReadPixels(TArray<FColor> &BitMap, UTextureRenderTarget2D* CaptureRenderTargetIns) const;

	UFUNCTION(BlueprintCallable)
		bool SaveCaptureToDisk(const FString& FileName, RenderTargetType RenderT) const;

	UFUNCTION(BlueprintCallable)
		void PrintHello();

	UFUNCTION(BlueprintCallable)
		bool RenderThreadSaveCaptureToDisk(const FString& FilePathT, const FString& FileName, RenderTargetType RenderT, int IWidth, int IHeight) const;

	/*static bool Save(const FString& FileName, UTextureRenderTarget2D* CaptureRenderTargetDesier);*/
	
	
	
};
