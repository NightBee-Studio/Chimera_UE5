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
	EBMt_Clay_A,
	EBMt_Clay_B,
	EBMt_Clay_C,
	EBMt_Clay_D,
	EBMt_Forest_A,	//5
	EBMt_Forest_B,	
	EBMt_Grass_A,
	EBMt_Grass_B,		
	EBMt_Grass_C,
	EBMt_Moss_A,	//10		
	EBMt_Moss_B,	
	EBMt_Mountain_A,
	EBMt_Mountain_B,
	EBMt_Mountain_C,
	EBMt_Mountain_D,//15
	EBMt_Mountain_E,
	EBMt_Mountain_F,
	EBMt_Sand_A,
	EBMt_Soil_A,
	EBMt_Soil_B,
	EBMt_Soil_C,
	EBMt_Soil_D,
	EBMt_Soil_E,
EBMt_Max,
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
	ETM_Moisture,
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

struct FTsBiomeModel  ;
struct FTsBiomeModels ;
class TsBiomeGroup
{
private:
	TArray<TsBiome*>	mBiomes ;

	FVector2D			mMin, mMax;
	FTransform			mTransform;

	int					mID ;
	FTsBiomeModel*		mModel  ;
	FVector2D			mHeightRange ;

public:
	static bool TryDone( TsBiome* b );
	static bool CheckDone( TsBiome* b );
	static void ClearDone() ;
	static void Done( TsBiome* b );
public:
	TsBiomeGroup( TsBiome* b, FTsBiomeModels& biome_models ) ;

	bool		IsInside( const FVector2D& p ) const ;
	float		GetMask(const FVector2D& p ) const ;
	//int			GetSeqID() const { return mSeqID ;}
	float		GetPixel(ETextureMap mp, const FVector2D& p) const ;
} ;

