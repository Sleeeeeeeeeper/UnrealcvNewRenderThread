// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include <memory>
#include <vector>
#include "Engine.h"
#include "Engine/TextureRenderTarget.h"
#include "Deps.h"

typedef uint64_t TTimePoint;

class RenderRequest : public FRenderCommand
{
public:
	struct RenderParams {
		UTextureRenderTarget2D* render_target;
		bool pixels_as_float;
		bool compress;

		RenderParams(UTextureRenderTarget2D* render_target_val, bool pixels_as_float_val, bool compress_val)
			: render_target(render_target_val), pixels_as_float(pixels_as_float_val), compress(compress_val)
		{
		}
	};
	struct RenderResult {
		TArray<uint8> image_data_uint8;
		TArray<float> image_data_float;

		TArray<FColor> bmp;
		TArray<FFloat16Color> bmp_float;

		int width;
		int height;
	};

private:
	static FReadSurfaceDataFlags setupRenderResource(const FTextureRenderTargetResource* rt_resource, const RenderParams* params, RenderResult* result, FIntPoint& size);
	bool use_safe_method_;

	std::shared_ptr<RenderParams> *params_;
	std::shared_ptr<RenderResult> *results_;

	std::shared_ptr<WorkerThreadSignal> wait_signal_;

	unsigned int req_size_;



public:
	RenderRequest(bool use_safe_method = false);
	~RenderRequest();

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
		ExecuteTask();
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(RenderRequest, STATGROUP_RenderThreadCommands);
	}

	// read pixels from render target using render thread, then compress the result into PNG
	// argument on the thread that calls this method.
	void getScreenshot(std::shared_ptr<RenderParams> params[], std::vector<std::shared_ptr<RenderResult>>& results, unsigned int req_size);

	void ExecuteTask();
};