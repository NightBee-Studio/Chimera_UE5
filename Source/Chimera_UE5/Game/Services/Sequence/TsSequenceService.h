// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "System/TsSystem.h"

#include "TsSequenceService.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, Abstract)
class CHIMERA_UE5_API UTsSequenceService
	: public UTsService
{
	GENERATED_BODY()

public:
	virtual void Initialize(USubsystem* owner, FSubsystemCollectionBase& collection);

	virtual void Deinitialize();
};
