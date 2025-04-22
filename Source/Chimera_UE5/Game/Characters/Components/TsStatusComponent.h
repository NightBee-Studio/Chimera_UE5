// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TsStatusComponent.generated.h"


UENUM(BlueprintType)
enum class ECharaStatus : uint8 {
	None = 0,
	ESch_SysHalt	UMETA(DisplayName = "SysHalt"	),
	ESch_SysInit	UMETA(DisplayName = "SysInit"	),
	ESch_SysCreate	UMETA(DisplayName = "SysCreate"	),
	ESch_SysDestroy	UMETA(DisplayName = "SysDestroy"),
	ESch_Invincible	UMETA(DisplayName = "Invincible"),
	ESch_Damaged	UMETA(DisplayName = "Damaged"	),
	ESch_Attacked	UMETA(DisplayName = "Attacked"	),
	ESch_Die		UMETA(DisplayName = "Die"		),
	ESch_Stun		UMETA(DisplayName = "Stun"		),
	ESch_OnGround	UMETA(DisplayName = "OnGround"	),
	ESch_OnHole		UMETA(DisplayName = "OnHole"	),
	ESch_OnWater	UMETA(DisplayName = "OnWater"	),
	ESch_OnLava		UMETA(DisplayName = "OnLava"	),
	ESch_Poison		UMETA(DisplayName = "Poison"	),
	ESch_Pararise	UMETA(DisplayName = "Pararise"	),
	Num	UMETA(Hidden)
};
ENUM_CLASS_FLAGS(ECharaStatus);



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHIMERA_UE5_API UTsStatusComponent
	: public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTsStatusComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float dt, ELevelTick type, FActorComponentTickFunction* func) override;

private:
	TArray<int  >	mStatus  ;
	TArray<float>	mCounter ;
	bool			mDirty   ;

public:
	UFUNCTION(BlueprintCallable)
	void	Set(ECharaStatus s, float counter=0.0f );
	UFUNCTION(BlueprintCallable)
	void	Sets(const TArray<ECharaStatus>& params, float counter = 0.0f);

	UFUNCTION(BlueprintCallable)
	void	Reset(ECharaStatus s);
	UFUNCTION(BlueprintCallable)
	void	Resets(const TArray<ECharaStatus>& params);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	bool	Is(ECharaStatus status) const ;
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	bool	Or(const TArray<ECharaStatus>& params) const;
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	bool	And(const TArray<ECharaStatus>& params) const;
};
