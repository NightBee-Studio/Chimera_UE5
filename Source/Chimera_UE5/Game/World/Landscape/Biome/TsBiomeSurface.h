#pragma once

#include "CoreMinimal.h"

#include "TsBiome.h"
#include "TsBiomeMap.h"


// -------------------------------- BiomeSurface  --------------------------------
struct TsOp;

class	TsBiomeSFunc
	: public TsNoiseMap {
protected:
	float			mHeight;
public:
	TsBiomeSFunc( const TsNoiseParam& cnf, float h = 1)
		: TsNoiseMap(cnf), mHeight(h) {}

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
protected:
	struct Layer {						// maybe need to be refactored....
		EMaterialType		mMaterialType;
		float				mOccupancy;
		TsOp*				mOp;
		float				mMin, mMax;	// this will be updated...
	};
	struct Config {						// maybe need to be refactored....
		EBiomeMapType		mMapSource;
		TArray<Layer>		mLayers;
	};
	TArray<Config>			mConfigs;

public:
	TsBiomeMFunc(TArray<Config> cnfs)
		: mConfigs(cnfs) {}

	TsMaterialValue	GetMaterial(const FVector2D& p);
};




// -------------------------------- Surface  --------------------------------

class	TsBiomeSurface {
public:
	//private:
	TArray<TsBiomeSFunc*>	mSFuncs;
	TArray<TsBiomeMFunc*>	mMFuncs;

public:
	TsBiomeSurface(
		TArray<TsBiomeSFunc*> s_funcs,
		TArray<TsBiomeMFunc*> m_funcs )
		: mSFuncs(s_funcs), mMFuncs(m_funcs) {}

public:
	float			GetHeight  (TsBiome* b, const FVector2D& p);	// world-coord
	TsMaterialValue	GetMaterial(TsBiome* b, const FVector2D& p);	// world-coord
	void			UpdateRemap(TsBiome* b, const FVector2D& p);	// world-coord
};

