#pragma once


#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"

#include "../Util/TsVoronoi.h"

#include "TsBiome.generated.h"



// -------------------------------- TsBiome  --------------------------------

UENUM(BlueprintType)
enum class EBiomeSType : uint8 {	// Surface(Height)
	E_SurfNone,	// out of continent
	E_SurfLake,
	E_SurfField	,
	E_SurfMountain,
};

UENUM(BlueprintType)
enum class EBiomeMType : uint8 {	// Moist(Water)
	E_Soil,
	E_Field,	//Field
	E_Tree,
	E_Forest,
};

UENUM(BlueprintType)
enum class EBiomeGType : uint8 {	// Genre
	E_GenreTropical,
	E_GenreDessert,
	E_GenreWilderness,
	E_GenreSavanna,
};


enum EMaterialType {
	MT_None,		//0
	MT_BaseLand,
	MT_LakeSoil_A,	//2
	MT_Soil_A,
	MT_Soil_B,		//4
	MT_Sand_A,
	MT_Sand_B,		//6
	MT_Snow_A,		
	MT_Grass_A,		//8
	MT_Grass_B,		
	MT_Forest_A,	//10
	MT_Forest_B,	
	MT_Rock_A,		//12
	MT_Moss_A,		
	MT_Moss_B,		//14
};


// -------------------------------- TsBiomeItem  --------------------------------
struct TsBiomeItem {
	float		mRatio;
	float		mThreshold;
};

struct TsBiomeItem_MType : public TsBiomeItem {
	EBiomeMType	mItem;
};

struct TsBiomeItem_SType : public TsBiomeItem {
	EBiomeSType	mItem;
};

struct TsBiomeItem_Material	: public TsBiomeItem {
	EMaterialType	mItem;
};



template <typename T>
concept DerivedFromTsFVector2D = std::is_base_of_v<FVector2D, T>;

template <typename T>
concept DerivedFromTsBiomeItem = std::is_base_of_v<TsBiomeItem, T>;






USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FTsBiomeModel
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Material"))
	TObjectPtr<UMaterialInstanceConstant>	mMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Textures"))
	TArray<TObjectPtr<UTexture2D>>			mTextures;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Mask"))
	TObjectPtr<UTexture2D>					mMask;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "HeightMap"))
	TObjectPtr<UTexture2D>					mHeightMap;
};

USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FTsBiomeSpec
	: public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "TypeSurf"))
	EBiomeSType				mSType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "TypeMoist"))
	EBiomeMType				mMType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "TypeGenre"))
	EBiomeGType				mGType;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "EItemFlag", DisplayName = "Flag"))
	//int32					mFlag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Models"))
	TArray<FTsBiomeModel>	Models;
};

//
// Surface materail must be changed for following parameters.
//	- Height
//  - Moist(Water?),
//  - Genre (Atmosphere?)
//

// -------------------------------- Biome --------------------------------
//
//
//

class TsBiome
	: public TsVoronoi
{
//private:
public:
	EBiomeSType		mSType;
	EBiomeMType		mMType;
	EBiomeGType		mGType;// ç°âÒÇÕÇ»Çµ

public:
	TsBiome(float x, float y);
	virtual ~TsBiome() {}

	void			SetMType(EBiomeMType ty)	{ mMType = ty; }
	EBiomeMType		GetMType()					{ return mMType; }
	void			SetSType(EBiomeSType ty)	{ mSType = ty; }
	EBiomeSType		GetSType()					{ return mSType; }
	void			SetOwner()					{ mOwner = this; }

	float			GetMask(const FVector2D& p);

	void 			GetBlend(TMap<TsBiome*,float>& array, const FVector2D& p);
};

