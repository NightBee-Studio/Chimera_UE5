#pragma once

#include "CoreMinimal.h"

#include "TsBiome.h"
#include "TsBiomeMap.h"


// -------------------------------- BiomeSurface  --------------------------------
struct TsOp;

enum EBiomeParamType
{
	EBPt_None = 0,
	EBPt_Height,
	EBPt_Scale,
	EBPt_Object,
} ;


union TsBiomeValue 
{
	float		f ;
	int			i ;
	UObject *	o ;
};

typedef TMap<EBiomeParamType,TsBiomeValue>	TsBiomeParams ;
typedef TArray<TsBiomeItem_Material>		TsBiomeMatItems ;

class	TsBiomeSFunc
	: public TsNoiseMap {
protected:
	float			mHeight;
	//TsBiomeParams	mParams ;
public:
	TsBiomeSFunc( const TsNoiseParam& cnf, float h
//const TsBiomeParams&	params 
)
		: TsNoiseMap(cnf)  
		, mHeight(h)	{}

public:
	virtual float	Remap(float val) const override { return  mHeight * (val - mMin) / (mMax - mMin); }

	// SurfFunc must access by GetHeight() not GetValue() ;
public:
	virtual void	RemapHeight(TsBiome* b, const FVector2D& p);
	virtual float	GetHeight  (TsBiome* b, const FVector2D& p) { return GetValue(p) ; }
};

#include "Surface/TsSurfField.h"
#include "Surface/TsSurfLake.h"
#include "Surface/TsSurfMountain.h"
#include "Surface/TsSurfNoise.h"


class	TsBiomeMFunc {
//protected:
public:
	ETextureMap						mMapType;
	TArray<TsBiomeItem_Material>	mItems;
	TsBiomeParams					mParams ;

public:
	TsBiomeMFunc(
			ETextureMap							type ,
			const TArray<TsBiomeItem_Material>&	items,
			const TsBiomeParams &				params
		): mMapType(type), mItems(items), mParams(params) {}
};




// -------------------------------- Surface  --------------------------------

class	TsBiomeSurface {
public:
	TArray<TsBiomeSFunc*>	mSFuncs;
	TArray<TsBiomeMFunc*>	mMFuncs;

public:
	TsBiomeSurface(
		TArray<TsBiomeSFunc*>	s_funcs,
		TArray<TsBiomeMFunc*>	m_funcs )
		: mSFuncs(s_funcs)
		, mMFuncs(m_funcs) {}

	float			GetHeight  (TsBiome* b, const FVector2D& p);	// world-coord
	//TsMaterialPixel	GetMaterial(TsBiome* b, const FVector2D& p);	// world-coord
	void			UpdateRemap(TsBiome* b, const FVector2D& p);	// world-coord
};

