// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "../TsSystem.h"
#include "TsWorldSetting.generated.h"



UCLASS(BlueprintType)
class CHIMERA_UE5_API ATsWorldSetting
	: public AWorldSettings
{
	GENERATED_BODY()
	
public:
	ATsWorldSetting(){
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Services", Meta = (DisplayName = "ServiceMode"))
	TEnumAsByte<EServiceMode>	mServiceMode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Services", Meta = (DisplayName="List of Services"))
	TArray<TObjectPtr<UObject>>	mServices;
} ;

