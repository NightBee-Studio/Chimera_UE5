// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"

#include "System/TsSystem.h"

#include "TsBulletService.generated.h"


UENUM(BlueprintType)
enum class EBulletType : uint8 {
	ETbu_None,

	ETbu_ArWood	UMETA(DisplayName = "ArWood"),
	ETbu_ArSteel,
	ETbu_ArFire,
	ETbu_ArBlast,

};

UENUM(BlueprintType, Meta = (Bitflags))
enum class EBulletFlag : uint8 {
	EFbu_Gravity,
	EFbu_GoStraightWithinRange,

	EFbu_HitBlow,
};
ENUM_CLASS_FLAGS(EBulletFlag);


USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FBulletSpec
	: public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Type"))
	EBulletType				mType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "EBulletFlag", DisplayName = "Flag"))
	int32					mFlag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Mesh"))
	TObjectPtr<UStaticMesh>	mMesh;

	//LangKey	mName;//name is now included in here
	//LangKey	mDescriptions;//desc is 
	//Params	mParams;
	//Sprite	mIcon;//icon
};


/**
 * 
 */
UCLASS(Blueprintable, Abstract)
class CHIMERA_UE5_API UTsBulletService
	: public UTsService
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "BulletService", meta = (RowType = "FBulletSpec"))
	TObjectPtr<UDataTable>		mSpecs;

	
};
