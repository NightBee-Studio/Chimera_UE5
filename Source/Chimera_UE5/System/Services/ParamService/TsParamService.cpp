// Fill out your copyright notice in the Description page of Project Settings.


#include "TsParamService.h"



void	UTsParamService::Initialize(USubsystem* owner, FSubsystemCollectionBase& collection)
{
	Super::Initialize(owner, collection);
}

void	UTsParamService::Deinitialize() 
{
	Super::Deinitialize();
}

void	UTsParamService::AddParams(FName name, UTsParams* params)
{
	mParamMaps.Add( name, params );
}

void	UTsParamService::RemoveParams(FName name)
{
	mParamMaps.Remove(name );
}
