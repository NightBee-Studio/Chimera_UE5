// Fill out your copyright notice in the Description page of Project Settings.

#include "TsAppKernel.h"
#include "System/TsSystem.h"

//
//UTsAppKernel::mAppStages =
//TMap<EAppStage, FString>
//		{// let subscene specified first
//			{ EAppStage::E_Title		,	FString("Title") },
//			//{ EAppStage::E_Guild		,	"Home"  },
//			//{ EAppStage::E_Game_Forest	,	"Game"  },
//		};
//


//
//void FTsTick::ExecuteTick(float dt, ELevelTick type, ENamedThreads::Type thread, const FGraphEventRef& ev)
//{
//	UE_LOG(LogTemp, Log, TEXT("FTsTick ExecuteTick"));
//}

#include "GameFramework/Actor.h"

class TsAppKernelImpl {
public:
	TsAppKernelImpl(FSubsystemCollectionBase& collection, TObjectPtr<UTsAppKernel> owner ) {

		//LoadObject<UTexture2D>(NULL, TEXT("/Game/Texture/T_sample00.T_sample00"), NULL, LOAD_None, NULL);
		UE_LOG(LogTemp, Log, TEXT("TsAppKernelImpl"));
	}
};


bool UTsAppKernel::IsTickable() const
{
	return true;
}

TStatId UTsAppKernel::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTsAppKernel, STATGROUP_Tickables);
}

void UTsAppKernel::Tick(float dt) 
{
	//TsAppKernelImpl* work = (TsAppKernelImpl*)mImplWork;
	for (UTsService* srv : mServices) {
		//sv->Update( 0 );
	}

	//UE_LOG(LogTemp, Log, TEXT("UTsAppKernel ExecuteTick"));
}

void UTsAppKernel::Initialize(FSubsystemCollectionBase& col)
{
	Super::Initialize(col);

	UE_LOG(LogTemp, Log, TEXT("UTsAppKernel Initialize"));

	//launch services
	mImplWork = new TsAppKernelImpl(col, this);

	for ( UBlueprint* bp : mServiceBPs ){
		UTsService* sv = bp->GeneratedClass->GetDefaultObject<UTsService>();
		mServices.Add( sv );
	}
	for ( UTsService *srv : mServices) {
		srv->Initialize(this, col);
	}
}

void UTsAppKernel::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("UTsAppKernel Deinitialize"));

	for (UTsService* srv : mServices) {
		srv->Deinitialize();
	}

	//delaunch services
	if (mImplWork != nullptr) {
		delete mImplWork;
		mImplWork = nullptr;
	}

	Super::Deinitialize();
}


void	UTsAppKernel::SetStage(EAppStage stage)
{
	if (mStage != stage) {
		mStage = stage;
	}
}

void UTsAppKernel::Load(EAppDataSlot slot)
{
	//mEvents[EVap_DataLoad].Broadcast();
}

void UTsAppKernel::Save(EAppDataSlot slot)
{
	//mEvents[EVap_DataSave].Broadcast();
}

