// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TsStateTreeData.generated.h"



USTRUCT()
struct CHIMERA_UE5_API FTsData_StateChara
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (DisplayName = "CharaStatus"))
	ECharaStatus mCharaStatus;
};

USTRUCT()
struct CHIMERA_UE5_API FTsData_DoTagState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (DisplayName = "StatusTag"))
	FGameplayTag mStatusTag;

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (DisplayName = "SkipTag"))
	FGameplayTag mSkipTag;
};


