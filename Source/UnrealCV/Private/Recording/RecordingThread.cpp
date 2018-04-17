#include "UnrealCVPrivate.h"
#include "RecordingThread.h"
#include "HAL/RunnableThread.h"
#include <thread>
#include <mutex>
#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TaskGraphInterfaces.h"
#include "SaveRenderCameraActor.h"
#include <iostream> 
#include <vector>
#include "Kismet/GameplayStatics.h"
#include "RenderRequest.h"
#include "Async/TaskGraphInterfaces.h"
#include "UE4CVServer.h"

#include "HighResScreenshot.h"
#include "Paths.h"

std::unique_ptr<FRecordingThread> FRecordingThread::instance_;

FRecordingThread::FRecordingThread()
	: stop_task_counter_(0), FormatedArgs({"","","","","","","","",})
{
    thread_.reset(FRunnableThread::Create(this, TEXT("FRecordingThread"), 0, TPri_BelowNormal)); // Windows default, possible to specify more priority
}


void FRecordingThread::startRecording(const TArray<FString>& Args)
{
    stopRecording();

    //TODO: check FPlatformProcess::SupportsMultithreading()?
    instance_.reset(new FRecordingThread());
		
/*
		UWorld* World = GEngine->GetWorld();
		TArray<AActor*> OutCams;
		UGameplayStatics::GetAllActorsOfClass(World, Camera->GetClass(), OutCams);
		instance_->MyCamera = Cast<AMyCameraActor>(OutCams[0]);*/
   // instance_->MyCaptureRenderTargetResource = Camera->CaptureRenderTarget->GameThread_GetRenderTargetResource();
		//instance_->CaptureRenderTargetDesiered = RenderTargetType::DefaultScene;
    
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("ThreadStartRec......"));
		//instance_->Run();
		

		
		
		UWorld* World = FUE4CVServer::Get().GetGameWorld();
		if (World != nullptr && instance_->FormatArgs(Args))
		{
		//GetTargetActor
		instance_->Tag = FName(*instance_->FormatedArgs[0]);
		UGameplayStatics::GetAllActorsWithTag(World, instance_->Tag, instance_->OutActors);
		if (instance_->OutActors[0] == nullptr)
		{
			//instance_->Stop();
			stopRecording();
		}
		instance_->MyCamera = Cast<ASaveRenderCameraActor>(instance_->OutActors[0]);

		
		//SetRenderType
		if (instance_->FormatedArgs[1] == "lit")
		{
			instance_->CaptureRenderTargetDesiered = RenderTargetType::DefaultScene;
		}
		else if (instance_->FormatedArgs[1] == "depth")
		{
			instance_->CaptureRenderTargetDesiered = RenderTargetType::Depth;
		}

		//SetFilePath
		instance_->FilePath = instance_->FormatedArgs[2];

		//SetDeltatime
		instance_->Timedelay = FCString::Atoi(*instance_->FormatedArgs[3]);
		
		//SetWidthAndHeight
		instance_->ImageWidth = FCString::Atoi((*instance_->FormatedArgs[4]));
		instance_->ImageHeight = FCString::Atoi((*instance_->FormatedArgs[5]));
		instance_->MyCamera->SizeX = instance_->ImageWidth;
		instance_->MyCamera->SizeY = instance_->ImageHeight;
		instance_->MyCamera->CaptureRenderTarget0->InitCustomFormat(instance_->ImageWidth, instance_->ImageHeight, PF_B8G8R8A8, true);
		instance_->MyCamera->CaptureRenderTargetDeep->InitCustomFormat(instance_->ImageWidth, instance_->ImageHeight, PF_B8G8R8A8, true);
	
		//SetImageNumber
		instance_->SaveNumCount = FCString::Atoi((*instance_->FormatedArgs[6]));

		

		instance_->is_ready_ = true;
		}

}

FRecordingThread::~FRecordingThread()
{
    stopRecording();
}

bool FRecordingThread::isRecording()
{
    return instance_ != nullptr;
}



void FRecordingThread::stopRecording()
{
    if (instance_)
    {
      instance_->EnsureCompletion();
      instance_.reset();
    }
}

/*********************** methods for instance **************************************/

bool FRecordingThread::Init()
{
    return true;
}

uint32 FRecordingThread::Run()
{
	int i = 0;
    while (stop_task_counter_.GetValue() == 0)
    {
        //make sire all vars are set up
        if (is_ready_ && (MyCamera!=nullptr)) {
                //image_capture_->getImages(settings_.requests, responses);
							FString FileName = FString::FromInt(i);
				//			MyCamera->SaveCaptureToDisk(FileName, instance_->CaptureRenderTargetDesiered);
							//RenderThread
							MyCamera->RenderThreadSaveCaptureToDisk(instance_->FilePath,FileName, instance_->CaptureRenderTargetDesiered,instance_->ImageWidth,instance_->ImageHeight);
							i++;
							//Sleep(1000);
							//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Sleep......"));
							_sleep(instance_->Timedelay);
							if (i>instance_->SaveNumCount)
							{
								instance_->Stop();
							}
        }
    }
    return 0;
}


void FRecordingThread::Stop()
{
    stop_task_counter_.Increment();
}


void FRecordingThread::Exit()
{

}

void FRecordingThread::EnsureCompletion()
{
    Stop();
    thread_->WaitForCompletion();
    //UAirBlueprintLib::LogMessage(TEXT("Stopped recording thread"), TEXT(""), LogDebugLevel::Success);
}

bool FRecordingThread::FormatArgs(const TArray<FString>& Args)
{
	instance_->FormatedArgs[0] = "None";
	instance_->FormatedArgs[1] = "lit";
	instance_->FormatedArgs[2] = FPaths::ProjectSavedDir();
	instance_->FormatedArgs[3] = "0";
	instance_->FormatedArgs[4] = "720";
	instance_->FormatedArgs[5] = "512";
	instance_->FormatedArgs[6] = "10000";

	if (Args.Num()>0)
	{
		instance_->FormatedArgs[0] = Args[0];
	}
	else 
	{ 
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("ParameterMissError"));
		return false;
	}

	if (Args.Num() > 1)
	{
		if (Args[1] == "lit" || Args[1] == "depth")
		{
			instance_->FormatedArgs[1] = Args[1];
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("ImageTypeError,OnlySupport lit or depth"));
			return false;
		}
	}
	else return true;

	if (Args.Num()>2)
	{
		if (FPaths::DirectoryExists(Args[2]))
		{
			instance_->FormatedArgs[2] = Args[2];
		}
		else 
		{ 
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("PathError"));
			return false;
		}
	}
	else return true;

	if (Args.Num() > 3)
	{
		
		if (0<= FCString::Atoi(*Args[3]) && FCString::Atoi(*Args[3]) <=100000)
		{
			instance_->FormatedArgs[3] = Args[3];
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("DeltimError,OnlySupport0-100000(ms)"));
			return false;
		}
	}
	else return true;

	if (Args.Num() > 4)
	{
		if (1 <= FCString::Atoi(*Args[4]) && FCString::Atoi(*Args[4]) <= 4096)
		{
			instance_->FormatedArgs[4] = Args[4];
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("ImageWidthError,OnlySupport1-4096"));
			return false;
		}
	}
	else return true;

	if (Args.Num() > 5)
	{
		if (1 <= FCString::Atoi(*Args[5]) && FCString::Atoi(*Args[5]) <= 4096)
		{
			instance_->FormatedArgs[5] = Args[5];
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("ImageHeightError,OnlySupport1-4096"));
			return false;
		}
	}
	else return true;

	if (Args.Num() > 6)
	{
		if (1 <= FCString::Atoi(*Args[6]) && FCString::Atoi(*Args[6]) <= 5000)
		{
			instance_->FormatedArgs[6] = Args[6];
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("CommandCorrect"));
			return true;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("ImageNumError,OnlySupport1-5000"));
			return false;
		}
	}
	else return true;

	//return false;
}
