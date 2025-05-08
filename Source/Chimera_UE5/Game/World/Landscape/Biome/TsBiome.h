#pragma once


#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"

#include "../Util/TsVoronoi.h"

#include "TsBiome.generated.h"



// -------------------------------- TsBiome  --------------------------------

UENUM(BlueprintType)
enum class EBiomeType : uint8 {
	BiomeTropicalField,	//Field
	BiomeTropicalMount,	//Mount
};

UENUM(BlueprintType)
enum class EBiomeSrfType : uint8 {
	SurfBase,

	SurfMountain,
	SurfField	,
	SurfLake	,
	SurfOcean	,	// out of continent
};

UENUM(BlueprintType)
enum class	EBiomeFuncType : uint8 {
	FuncField,
	FuncMountain,
	FuncTropicalForest,
	FuncDesert,
	FuncWilderness,
};



USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FBiomeSpec
	: public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Type"))
	EBiomeType				mType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "EItemFlag", DisplayName = "Flag"))
	int32					mFlag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Mesh"))
	TObjectPtr<UStaticMesh>	mMesh;
};




// -------------------------------- Biome --------------------------------
//
//
//

class TsBiome
	: public TsVoronoi
{
private:
	EBiomeType		mType;
	EBiomeSrfType	mSurfType;//Å@Ç¢ÇÁÇ»Ç¢
public:
	TsBiome(float x, float y) : TsVoronoi(x, y) {}
	virtual ~TsBiome() {}

	void			SetBiomeType(EBiomeType ty)	{ mType = ty; }
	EBiomeType		GetBiomeType()				{ return mType; }
	void			SetBiomeSrfType(EBiomeSrfType ty) { mSurfType = ty; }
	EBiomeSrfType	GetBiomeSrfType()			{ return mSurfType; }
};

typedef TArray<TsBiome*> TsBiomeGroup;

