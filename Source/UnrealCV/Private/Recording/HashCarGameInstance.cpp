// Fill out your copyright notice in the Description page of Project Settings.
#include "UnrealCVPrivate.h"
#include "HashCarGameInstance.h"
#include "RecordingThread.h"


void UHashCarGameInstance::Shutdown()
{
	while (FRecordingThread::isRecording())
	{
		FRecordingThread::stopRecording();
	}
	Super::Shutdown();
}

