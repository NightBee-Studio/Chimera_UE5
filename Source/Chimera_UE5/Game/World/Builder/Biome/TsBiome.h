#pragma once


#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"

#include "../Util/TsVoronoi.h"

#include "TsBiome.generated.h"



// -------------------------------- TsBiome  --------------------------------

UENUM(BlueprintType)
enum class EBiomeSType : uint8 {	// Surface(Height)
	EBSf_None,	// out of continent
	EBSf_Lake,
	EBSf_Field	,
	EBSf_Mountain,
};

UENUM(BlueprintType)
enum class EBiomeMType : uint8 {	// Moist(Water)
	EBMo_Soil,
	EBMo_Field,
	EBMo_Tree,
	EBMo_ForestA,
	EBMo_ForestB,
};

UENUM(BlueprintType)
enum class EBiomeGType : uint8 {	// Genre
	EBGn_A,
	EBGn_B,
	EBGn_C,
	EBGn_D,
};

UENUM(BlueprintType)
enum EMaterialType {
	EBMt_None,		//0
	EBMt_Soil_A,
	EBMt_Soil_B,
	EBMt_Soil_C,
	EBMt_Grass_A,		//8
	EBMt_Grass_B,		
	EBMt_Forest_A,	//10
	EBMt_Forest_B,	
	EBMt_Rock_A,		//12
	EBMt_Moss_A,		
	EBMt_Moss_B,		//14
};


UENUM(BlueprintType)
enum ETextureMap {
	ETM_Height,
	ETM_Genre,
	ETM_Flow,
	ETM_Wear,
	ETM_Slope,
	ETM_Deposite,
	ETM_Curvature,
} ;


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
concept DerivedFVector2D = std::is_base_of_v<FVector2D, T>;

template <typename T>
concept DerivedBiomeItem = std::is_base_of_v<TsBiomeItem, T>;





USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FTsBiomeModel
{
	GENERATED_BODY()
public:
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "EItemFlag", DisplayName = "Flag"))
	//int32							mFlag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Textures"))
	TMap<TEnumAsByte<ETextureMap>,TObjectPtr<UTexture2D>>
									mTextures;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Material"))
	TObjectPtr<UMaterialInstance>	mMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Model"   ))
	TObjectPtr<UStaticMesh>			mModel;
};

USTRUCT(BlueprintType)
struct CHIMERA_UE5_API FTsBiomeSpec
	: public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "TypeSurf"))
	EBiomeSType				mSType;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "TypeMoist"))
	//EBiomeMType				mMType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "TypeGenre"))
	EBiomeGType				mGType;

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

