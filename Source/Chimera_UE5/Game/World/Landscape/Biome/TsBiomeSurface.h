#pragma once

#include "CoreMinimal.h"

#include "TsBiome.h"
#include "TsBiomeMap.h"


// -------------------------------- BiomeSurface  --------------------------------

enum EOp {
	EOp_Set,
	EOp_Add,
	EOp_Sub,
	EOp_Max,
	EOp_Min,
};

class	TsBiomeSrfFunc
	: public TsNoiseMap {
protected:
	float			mHeight;
	EOp				mOp;
public:
	TsBiomeSrfFunc( const TsNoiseParam& cnf, float h = 1)
		: TsNoiseMap(cnf), mHeight(h), mOp(EOp_Add) {}

public:
	virtual float	Remap(float val) const override { return  mHeight * (val - mMin) / (mMax - mMin); }

	// SurfFunc must access by GetHeight() not GetValue() ;
public:
	virtual void	RemapHeight(TsBiome* b, const FVector2D& p);
	virtual float	GetHeight(TsBiome* b, const FVector2D& p) { return GetValue(p) ; }
};

#include "Surface/TsSurfField.h"
#include "Surface/TsSurfLake.h"
#include "Surface/TsSurfMountain.h"
#include "Surface/TsSurfNoise.h"

struct TsOp;

class	TsBiomeMatFunc {
protected:
	struct Layer {
		EMaterialType		mMaterialType;
		float				mOccupancy;
		TsOp*				mOp;
		float				mMin, mMax;	//// this will be updated...
	};
	struct Config {
		EBiomeMapType		mMapSource;
		TArray<Layer>		mLayers;
	};
	TArray<Config>			mConfigs;

public:
	TsBiomeMatFunc(TArray<Config> cnfs)
		: mConfigs(cnfs) {
	}
	~TsBiomeMatFunc() {
	}

	TsMaterialValue	GetMaterial(const FVector2D& p);
};


class	TsBiomeSurface {
public:
//private:
	TArray<TsBiomeSrfFunc*>	mSurfaceFuncs;
	TArray<TsBiomeMatFunc*>	mMaterialFuncs;
	
	//TsImageMap<float>*		mMask;			// going to be depricated

	//TArray<TsBiomeGroup>	mGroups;		// going to be depricated
	//int						mGroupNum;		// going to be depricated

public:
	TsBiomeSurface(
		  TArray<TsBiomeSrfFunc*> s_funcs
		, TArray<TsBiomeMatFunc*> m_funcs)
		: mSurfaceFuncs(s_funcs)
		, mMaterialFuncs(m_funcs)
		//, mMask(nullptr)
		//, mGroupNum(group_num)	
	{}

public:
	//TsImageMap<float>*		CreateMask(int reso, const FBox2D& bound);

	float					GetHeight  ( TsBiome *b, const FVector2D& p );		// world-coord
	TsMaterialValue			GetMaterial( TsBiome* b, const FVector2D& p );	// world-coord
	void					RemapHeight(TsBiome* b, const FVector2D& p);	// world-coord

	//void					ForeachGroup();

	//void					GatherBiome(TsBiome* b);
};
