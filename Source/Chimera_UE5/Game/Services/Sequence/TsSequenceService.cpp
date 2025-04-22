// Fill out your copyright notice in the Description page of Project Settings.


#include "TsSequenceService.h"


void UTsSequenceService::Initialize(USubsystem* owner, FSubsystemCollectionBase& collection)
{
	UE_LOG(LogTemp, Log, TEXT("UTsSequenceService Initialize") );

}

void UTsSequenceService::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("UTsSequenceService Deinitialize"));
}

