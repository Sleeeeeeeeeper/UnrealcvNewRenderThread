#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include <memory>
#include "SaveRenderCameraActor.h"
#include "TextureResource.h"

class FRecordingThread : public FRunnable
{
public:
	FRecordingThread();
	virtual ~FRecordingThread();

	static void startRecording(const TArray<FString>& Args);
	static void stopRecording();
	static bool isRecording();



protected:
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

private:
	void EnsureCompletion();
	bool FormatArgs(const TArray<FString>& Args);


	TArray<FString> FormatedArgs;

		TArray<AActor*> OutActors;

		FName Tag;

		int Timedelay;
		int ImageWidth;
		int ImageHeight;
		int SaveNumCount;
		
		ASaveRenderCameraActor* MyCamera;

		FString FilePath;

		FTextureRenderTargetResource* MyCaptureRenderTargetResource;

		UTextureRenderTarget2D* MyCaptureRenderTarget;

		RenderTargetType CaptureRenderTargetDesiered;

		FThreadSafeCounter stop_task_counter_;
    
    static std::unique_ptr<FRecordingThread> instance_;

    std::unique_ptr<FRunnableThread> thread_;

    bool is_ready_ = false;

		
};