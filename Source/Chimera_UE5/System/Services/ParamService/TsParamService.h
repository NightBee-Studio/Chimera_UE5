// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TsParam.h"
#include "../TsService.h"
#include "TsParamService.generated.h"

//
// SaveData for local-save
// Net Transfer for DB.
// 
// 
// 
//


UENUM(BlueprintType)
enum EDataMode : uint8 {
	EMda_LocalMode,
		EMda_SystemSlot,
		EMda_GameSlot,

	EMda_RemoteMode,
		EMda_PlayerSlot,
};


USTRUCT()
struct CHIMERA_UE5_API FTsData
	: public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Data", meta = (DisplayName = "Text"))
	FText		mText;
	//UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Data", meta = (DisplayName = "Voice"))
	//	void  *		mVoice;
};


UCLASS(Blueprintable)
class CHIMERA_UE5_API UTsParamService
	: public UTsService
{
	GENERATED_BODY()

private:
	void*		mImplWork;

public:
	UTsParamService(){}

	//param maps
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (DisplayName = "Params"))
	TMap<FName, TObjectPtr<UTsParams>>	mParamMaps;

	FORCEINLINE UTsParams*	GetParams(FName name) const { return mParamMaps.Contains(name) ? mParamMaps[name] : nullptr; }
	void					AddParams(FName name, UTsParams* param);
	void					RemoveParams(FName name);


	virtual void	Initialize( USubsystem* owner, FSubsystemCollectionBase& collection) override;
	virtual void	Deinitialize() override;

	virtual void	Update( EServiceFlag f ) override {}
};

