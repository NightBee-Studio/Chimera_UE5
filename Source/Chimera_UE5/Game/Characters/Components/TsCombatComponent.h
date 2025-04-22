// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TsCombatComponent.generated.h"


UENUM(BlueprintType)
enum class ECombatType : uint8 {
	None				UMETA(Hidden),

	E_Physical			UMETA(Hidden),
		E_PhyWeapon		UMETA(DisplayName = "Phys Weapon"	),
		E_PhyCrush		UMETA(DisplayName = "Phys Crush"  ),
		E_PhyPush		UMETA(DisplayName = "Phys Push"   ),

	E_Magic				UMETA(Hidden),
		E_MgFlame		UMETA(DisplayName = "Magic Flame"	),

	E_Other				UMETA(Hidden),
		E_Poison		UMETA(DisplayName = "Poison"	),
		E_Pararise		UMETA(DisplayName = "Pararise"	),

	Num	UMETA(Hidden)
};
//ENUM_CLASS_FLAGS(ECombatType);

UENUM(BlueprintType)
enum class ECombatFlag : uint8 {
	None = 0,
	E_DDdDD,
};
ENUM_CLASS_FLAGS(ECombatFlag);




USTRUCT(BlueprintType)
struct FCombatValue {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "CombatParam")
	ECombatType		mType;
	UPROPERTY(BlueprintReadWrite, Category = "CombatParam")
	int				mValue;
};

USTRUCT(BlueprintType)
struct FOffenseParam {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "CombatParam")
	TArray<FCombatValue>	mValues;
};

USTRUCT(BlueprintType)
struct FDefenseParam {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "CombatParam")
	TArray<FCombatValue>	mValues;
};


USTRUCT(BlueprintType)
struct FCombat {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "CombatParam")
	ECombatFlag			mFlag;

	//EnumHasAnyFlags(aBitFlagEnum, EBitFlagEnum::One);
	//	bool aResultOR	= EnumHasAnyFlags(aBitFlagEnum, EBitFlagEnum::One);
	//	bool aResultAND = EnumHasAllFlags(aBitFlagEnum, EBitFlagEnum::One);

	//TObject<ACharacter>	mOffense
	FOffenseParam		mOffenseParam;

	//TObject<ACharacter>	mDefense
	FDefenseParam		mDefenseParam;

	FVector				mDir;
	FVector				mPoint;
	FVector				mNormal;
};




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHIMERA_UE5_API UTsCombatComponent
	: public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTsCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float dt, ELevelTick type, FActorComponentTickFunction* func) override;
};


