// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"

#include "System/TsSystem.h"

#include "TsItemService.generated.h"


UENUM(BlueprintType,Meta=(Bitflag))
enum class EItemType : uint8 {
	ETit_None UMETA(DisplayName = "None"),
	//weapon
	ETit_WP_SwordA,
	ETit_WP_SwordB,
	ETit_WP_SwordC,
	ETit_WP_SwordD,
};

UENUM(BlueprintType, Meta = (Bitflags))
enum class EItemFlag : uint8 {
	E_Weapon	,
	E_Equipment	,
	E_Item		,

	E_Usable	,
	E_Countable	,
};
ENUM_CLASS_FLAGS(EItemFlag);

UENUM(BlueprintType, Meta = (Bitflags))
enum class EItemStatus : uint8 {
	E_Equipped		UMETA(DisplayName = "Equipped"),
	E_Stocked		UMETA(DisplayName = "Stocked"),
	E_Used			UMETA(DisplayName = "Used"),

	E_ExecPick	,
	E_ExecDrop	,
	E_ExecUse	,
	E_SysInit	,
	E_SysAdd	,
	E_SysDelete	,
};
ENUM_CLASS_FLAGS(EItemStatus);


USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FItemSpec 
	: public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Type"))
	EItemType				mType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask,BitmaskEnum = "EItemFlag",DisplayName = "Flag"))
	int32					mFlag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Mesh"))
	TObjectPtr<UStaticMesh>	mMesh;

	//LangKey	mName;//name is now included in here
	//LangKey	mDescriptions;//desc is 
	//Params	mParams;
	//Sprite	mIcon;//icon
}; 

UCLASS(Blueprintable)
class CHIMERA_UE5_API UItemData 
	: public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Type"))
	EItemType		mType;	// Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Value"))
	int				mValue;	// Durable(F_Durable), Time(F_Time), Mattock:Digging time, Num(F_Countable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "EItemStatus", DisplayName = "Status"))
	EItemStatus		mStatus;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Seed Main"))
	int				mSeed0;	// main seed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Seed Sub"))
	int				mSeed1;	// sub seed

	//-------------------------------------------------------------------------
	//bool  IsStatus(EItemStatus s)		{ return (mStatus & (int)s) > 0; }
	//void  SetStatus(EItemStatus s)		{ mStatus |= (int)s; }
	//void  ResetStatus(EItemStatus s)	{ mStatus &= ~(int)s; }

	//bool  IsFlag(EItemFlag f) { return ItemSpec.Get(mType).IsFlag(f); }
};



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHIMERA_UE5_API UTsItemComponent
	: public UActorComponent
{
	GENERATED_BODY()
public:
	UTsItemComponent() {}

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemComponent", Meta = (DisplayName = "Data"))
	TObjectPtr<UItemData>	mData;
};






UCLASS(Blueprintable,Abstract)
class CHIMERA_UE5_API UTsItemService
	: public UTsService
{
	GENERATED_BODY()

	void*									mImplWork;

public:
	UTsItemService();

	UPROPERTY(BlueprintReadOnly,EditAnywhere, Meta = (RowType = "FItemSpec"))
	TObjectPtr<UDataTable>					mSpecs;

	UPROPERTY(BlueprintReadOnly,EditAnywhere	)
	TObjectPtr<UObject>						mTemplate;

	UPROPERTY(BlueprintReadOnly,VisibleAnywhere	)
	TArray<TObjectPtr<UTsItemComponent>>	mItems;
	

	virtual void	Initialize(USubsystem* owner, FSubsystemCollectionBase& collection);
	virtual void	Deinitialize();

//	virtual void	Update(EAppServiceFlag f);

public:
//	void Sync();

//	FItem			Find(ItemType ty, ItemStatus st = ItemStatus.S_None);
//	TArray<FItem>	FindAll(ItemType ty, ItemStatus st = ItemStatus.S_None);
};
