// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "System/TsSystem.h"

#include "TsAppKernel.generated.h"



// AppKernel
//	applicaiton kernel...
//	
//	scene managment        ( loading fade-in fade-out )
//	save data              ( parameters save/load )
//	controlling services   ( network, sound, language, ads )
//
//  OnApplicationQuit() is done.
//

// Once AppKernel is accessed by other service, it will be automatically launched.
// and will be the persisitant object.



UENUM(BlueprintType)
enum class EAppStatus : uint8 {
	ESap_Boot			UMETA(DisplayName = "Boot"),
	ESap_TitleLoad		UMETA(DisplayName = "TitleLoad" ),
	ESap_Title			UMETA(DisplayName = "Title"     ),
	ESap_GuildLoad		UMETA(DisplayName = "GuildLoad" ),
	ESap_Guild			UMETA(DisplayName = "Guild"     ),
	ESap_GameLoad		UMETA(DisplayName = "GameLoad"  ),
	ESap_Game			UMETA(DisplayName = "Game"      ),
};

UENUM(BlueprintType)
enum class EAppStage : uint8 {
	Eap_Title,
	Eap_Guild,

	Eap_Game_Forest,
	Eap_Game_Paddle,
	Eap_Game_Mountain,
	Eap_Game_Artifact,
};

UENUM(BlueprintType)
enum EAppEvent {
	EVap_DataLoad,
	EVap_DataSave,
};

UENUM(BlueprintType)
enum EAppDataSlot {
	Eap_SystemSlot,
	Eap_GameSlot,
};


UCLASS(Blueprintable,Abstract)
class CHIMERA_UE5_API UTsAppKernel
	//: public UEngineSubsystem
	: public UGameInstanceSubsystem
	, public FTickableGameObject
	, public TsSingleton<UTsAppKernel>
{
	GENERATED_BODY()
	
	//Internal Parameters ----------------------------------------------------
private:
	void* mImplWork;

public:
	UTsAppKernel() 
		: TsSingleton<UTsAppKernel>(this)
	{
		mImplWork = nullptr;
		mStatus = EAppStatus::ESap_Boot;
	}

	//Unreal Engine ----------------------------------------------------
public:
	// Subsystem Inheritance
	virtual void	Initialize(FSubsystemCollectionBase& collection) override;
	virtual void	Deinitialize() override;

	// TickObject Inheritance
	virtual bool	IsTickable() const override;
	virtual			TStatId GetStatId() const override;
	virtual void	Tick(float DeltaTime) override;


	// Events ----------------------------------------------------
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKernelDelegate);
//	UPROPERTY(BlueprintAssignable, Category = "AppKernel", Meta = (DisplayName = "Events"))
	//UPROPERTY(Category = "AppKernel", Meta = (DisplayName = "Events"))
	//TArray<FKernelDelegate>			mEvents;

	// Parameter ----------------------------------------------------
public:
	// System Status
	UPROPERTY(BlueprintReadOnly, Category = "AppKernel", Meta = (DisplayName = "Status"))
	EAppStatus						mStatus;

	// System Services
	UPROPERTY(BlueprintReadOnly, EditAnywhere   , Category = "AppKernel", Meta = (DisplayName = "Service BPs"))
	TArray<TObjectPtr<UBlueprint>>	mServiceBPs;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "AppKernel", Meta = (DisplayName = "Services"   ))
	TArray<TObjectPtr<UTsService>>	mServices;

	// Stage 
	//UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "AppKernel", Meta = (DisplayName = "StageSet"   ))
	//TMap<int, UMap>					mStageSet;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "AppKernel", Meta = (DisplayName = "Stage"      ))
	EAppStage						mStage;


	// Funtions ----------------------------------------------------
public:
	void			SetStage(EAppStage stage ) ;

	void			Load( EAppDataSlot slot ) ;
	void			Save( EAppDataSlot slot ) ;

	
	template<typename T>
	static T*		GetService() { 
		if ( UTsAppKernel* kernel = Instance()){
			for (auto s : kernel->mServices) {
				if (s && s->IsA(T::StaticClass())) return Cast<T>(s);
			}
		}
		return nullptr;
	}
};
