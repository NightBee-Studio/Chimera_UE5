// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/Subsystem.h"

#include "System/TsSystem.h"

#include "TsGameSubsystem.generated.h"

	//: public FTickableGameObject

// using the thread control for any tasks.
// 
//::ParallelFor(2, [this](const int32 Index){
//		if (Index != 0){
//			for (int i = 0; i < 1000; i++){
//				PrintString();
//			}
//		}
//	});




UCLASS(Blueprintable,Abstract)
class CHIMERA_UE5_API UTsGameSubsystem
	: public UWorldSubsystem
	, public FTickableGameObject
	, public TsSingleton<UTsGameSubsystem>
{
	GENERATED_BODY()

	void*			mImplWork;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameManager", Meta = (DisplayName = "Service BPs"))
	TArray<TObjectPtr<UBlueprint>>		mServiceBPs;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "AppKernel", Meta = (DisplayName = "Services"))
	TArray<TObjectPtr<UTsService>>		mServices;


	virtual void	Initialize(FSubsystemCollectionBase& collection);
	virtual void	Deinitialize();

	virtual bool	IsTickable() const override;
	virtual			TStatId GetStatId() const override;
	virtual void	Tick(float DeltaTime) override;

public:
	template<typename T>
	T* GetService(UObject* context) {
		if (UTsGameSubsystem* sys = Instance()) {
			for (auto s : sys->mServices) {
				if (s && s->IsA(T::StaticClass())) return Cast<T>(s);
			}
		}
		return nullptr;
	}
};
