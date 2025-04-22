// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TsSkillComponent.generated.h"


UENUM(BlueprintType)
enum class ESkillType : uint8 {
	None = 0,
		E_PhyWeapon		UMETA(DisplayName = "Phys Weapon"),
		E_PhyCrush		UMETA(DisplayName = "Phys Crush" ),
		E_PhyPush		UMETA(DisplayName = "Phys Push"  ),
	Num	UMETA(Hidden)
};
//ENUM_CLASS_FLAGS(ESkillType);


//USTRUCT(BlueprintType)
//struct FCombat {
//	GENERATED_BODY()
//
//	UPROPERTY(BlueprintReadWrite, Category = "CombatParam")
//	ECombatFlag			mFlag;
//
//	//UPROPERTY(BlueprintReadWrite, Category = "CombatParam")
//	//ACharacter			mOffense
//	UPROPERTY(BlueprintReadWrite, Category = "CombatParam")
//	FOffenseParam		mOffenseParam;
//
//	//UPROPERTY(BlueprintReadWrite, Category = "CombatParam")
//	//ACharacter			mDefense
//	UPROPERTY(BlueprintReadWrite, Category = "CombatParam")
//	FDefenseParam		mDefenseParam;
//
//	UPROPERTY(BlueprintReadWrite, Category = "CombatParam")
//	FVector				mDir;
//	UPROPERTY(BlueprintReadWrite, Category = "CombatParam")
//	FVector				mPoint;
//	UPROPERTY(BlueprintReadWrite, Category = "CombatParam")
//	FVector				mNormal;
//};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHIMERA_UE5_API UTsSkillComponent
	: public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTsSkillComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float dt, ELevelTick type, FActorComponentTickFunction* func) override;
};


