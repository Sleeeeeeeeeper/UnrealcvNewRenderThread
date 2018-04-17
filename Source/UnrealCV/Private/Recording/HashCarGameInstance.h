// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HashCarGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class UHashCarGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
protected:
	virtual void Shutdown();
	
	
};
