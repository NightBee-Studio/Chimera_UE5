#pragma once

#include "CoreMinimal.h"

#include "TsBiome.h"
#include "TsBiomeMap.h"


// -------------------------------- BiomeSurface  --------------------------------

class	TsBiomeSrfFunc : public TsNoiseMap {
protected:
	float			mHeight;

public:
	TsBiomeSrfFunc(const TsNoiseParam& cnf, float h = 1)
		: TsNoiseMap(cnf), mHeight(h) {}

public:
	virtual void	Exec_EachGroup(TsBiomeGroup& grp) {}
	virtual void	Exec_UpdateRemap(const FVector2D& p) {}

	virtual float	Remap(float val) const override { return  mHeight * (val - mMin) / (mMax - mMin); }

public:
	// SurfFunc must access by GetHeight() not GetValue() ;
	virtual float	GetHeight(TsBiome* b, const FVector2D& p){ return GetValue(p) ;	}
};

#include "Surface/SurfField.h"
#include "Surface/SurfLake.h"
#include "Surface/SurfMountain.h"
#include "Surface/SurfNoise.h"

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

	static
	TArray<TsBiomeMatFunc*>	gList;

public:
	TsBiomeMatFunc(TArray<Config> cnfs)
		: mConfigs(cnfs) {
		gList.Add(this);
	}
	~TsBiomeMatFunc() {
		gList.Remove(this);
	}

	static
	void			UpdateOccupancy();

	TsMaterialValue	GetMaterial(const FVector2D& p);
};


class	TsBiomeSurface {
public:
	enum Flag {
		FL_None			= 0,
		FL_BaseHeight	= (1 << 0),
		FL_BaseMaterial = (1 << 1),
		FL_BaseShape	= (1 << 2),
	};

public:
//private:
	Flag					mFlag;

	TArray<TsBiomeSrfFunc*>	mSurfaceFuncs;
	TArray<TsBiomeMatFunc*>	mMaterialFuncs;
	
	TsImageMap<float>*		mMask;

	TArray<TsBiomeGroup>	mGroups;
	int						mGroupNum;

public:
	TsBiomeSurface(Flag f
		, int reso, const FBox2D& bound, const FString& name
		, int group_num
		, TArray<TsBiomeSrfFunc*> s_funcs
		, TArray<TsBiomeMatFunc*> m_funcs)
		: mFlag(f)
		, mSurfaceFuncs(s_funcs)
		, mMaterialFuncs(m_funcs)
		, mMask(nullptr)
		, mGroupNum(group_num)	{}

public:
	TsImageMap<float>*		CreateMask(int reso, const FBox2D& bound);

	float					GetHeight  ( TsBiome *b, const FVector2D& p );		// world-coord
	TsMaterialValue			GetMaterial( TsBiome* b, const FVector2D& p );	// world-coord

	void					UpdateRemap(const FVector2D& p);	// world-coord

	bool					IsBase(){ return mFlag & (FL_BaseHeight | FL_BaseMaterial) ;}

	void					ForeachGroup();

	void					GatherBiome(TsBiome* b);
};
