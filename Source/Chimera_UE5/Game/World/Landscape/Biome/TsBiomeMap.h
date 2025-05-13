#pragma once

#include "CoreMinimal.h"

#include "../Util/TsImageMap.h"



// -------------------------------- TsBiomeMaps  --------------------------------


enum EBiomeMapType {
	BiomeMapNone,	//nothing

	BiomeMapMoist,	//moisture
	BiomeMapTempr,	//temperture
	BiomeMapGenre,	//genre
	BiomeMapFlow,	//flow
	BiomeMapPond,	//Pond

	BiomeMapMountain,	//
	BiomeMapSlope,	//
	BiomeMapPlant,	//bigger = taller

	BiomeMapNormal,	//
};


template <typename T>
concept DerivedFromTsFVector2D = std::is_base_of_v<FVector2D, T>;

//	Hue Sat Val		=>	 	Genre Temp Moist
class TsBiomeMap
	: public TsImageMap<float>
	, public TsNoiseMap {

private:
	static
	TMap<EBiomeMapType, TsBiomeMap*>	gBiomeMaps;
public:
	static TsBiomeMap*	GetBiomeMap(EBiomeMapType ty	              ) { return gBiomeMaps[ty]; }
	static void			AddBiomeMap(EBiomeMapType ty, TsBiomeMap* map) { gBiomeMaps.Emplace( ty, map ); }



public:
	TsBiomeMap(int w, int h, const FBox2D* bound, const TsNoiseParam& cnf = TsNoiseParam())
		: TsImageMap<float>(w, h, bound)
		, TsNoiseMap(cnf) {}
	virtual ~TsBiomeMap() {}

	virtual float	RemapImage(float v, float range) const override;
	virtual float	GetValue(const FVector2D& p) override;
	virtual void	SetPixel(int x, int y, float v)override;

	// Items
public:
	struct Item {
		int		mItem;
		float	mRatio;
		float	mThresold;
	};

	template<DerivedFromTsFVector2D Tpoint>
	void SetupItems(TArray<Tpoint>& points, TArray<Item>& items) {
		TArray<float>	samples;
		for (const auto& p : points) samples.Add(GetValue(p));
		samples.Sort();		// sort to determine the ratio of the group.
		for (auto& it : items) {
			it.mThresold = samples[(samples.Num() - 1) * it.mRatio];
		}
	}

	template<DerivedFromTsFVector2D Tpoint>
	void SelectItem(const Tpoint& point, TArray<Item>& items, std::function<void(const Item&)>func) {
		float h = GetValue(point);
		for (auto& it : items) {
			if (h < it.mThresold) {
				func( it );
				break;
			}
		}
	}
};



class TsHeightMap
	: public TsBiomeMap 
{
public:
	TsHeightMap(int w, int h, const FBox2D* bound) : TsBiomeMap(w, h, bound) {}
	virtual ~TsHeightMap() {}

	void	SaveAll(int x, int y, int w, int h);

	FVector CalcNormal(
		float x, float y,// texel coordinate
		float grid_height = 1.0f,
		float grid_size = 1.0f);

	float	RemapImage(float v, float range ) const override;
};



//	Hue Sat Val		=>	 	Genre Temp Moist
class TsPlantMap
	: public TsBiomeMap 
{
public:
	TsPlantMap(int w, int h, const FBox2D* bound) : TsBiomeMap(w, h, bound) {}
	virtual ~TsPlantMap() {}

	float	GetValue(const FVector2D& p) override;
};

class TsGenreMap
	: public TsBiomeMap
{
public:
	TsGenreMap(int w, int h, const FBox2D* bound, const TsNoiseParam& cnf) : TsBiomeMap(w, h, bound, cnf) {}
	virtual ~TsGenreMap() {}

	float	GetValue(const FVector2D& p) override;
};

class TsMountMap
	: public TsBiomeMap 
{
public:
	TsMountMap(int w, int h, const FBox2D* bound);
	virtual ~TsMountMap() {}

	void	UpdateRemap(const FVector2D& p) override;
	float	GetValue(const FVector2D& p) override;
};




enum EMaterialType {
	MT_None,
	MT_OceanSoil_A,
	MT_LakeSoil_A,	//2
	MT_Soil_A,
	MT_Soil_B,		//4
	MT_Sand_A,
	MT_Sand_B,		//6
	MT_Grass_A,
	MT_Grass_B,		//8
	MT_Forest_A,
	MT_Forest_B,	//10
	MT_Rock_A,
	MT_Moss_A,		//12
	MT_Moss_B,
};

struct TsMaterialPixel {
	EMaterialType	mA;// , mB;
	float			mAlpha;//

	bool operator < (const TsMaterialPixel& a) const { return mAlpha < a.mAlpha; }// sorting be random value

	TsMaterialPixel(EMaterialType a, float al = 0) : mA(a), mAlpha(al) {}
};

struct TsMaterialValue
	: public TArray<TsMaterialPixel> 
{
	void Merge(const TsMaterialValue& a, float rate = 1);
};

class TsMaterialMap
	: public TsBiomeMap 
{
private:
	TMap<EMaterialType, TsImageMap<float>*>
					mDic;
	TsImageMap<int>	mIndexMap;	//RGBA
	TsImageMap<int>	mAlphaMap;	//RGBA

	TsImageMap<int>	mOutIndexMap;//RGBA
	TsImageMap<int>	mOutAlphaMap;//RGBA

public:
	TsMaterialMap(int w, int h, const FBox2D* bound);
	virtual ~TsMaterialMap() {}

	void			SetMaterialPixel(int x, int y) ;

	void			SetMaterial(int x, int y, const TsMaterialValue& mv);
	void			SaveAll(int x, int y, int w, int h);
};


class MarchingSquareMap {
	TsImageMap<float>	mMap ;
	
public:
	MarchingSquareMap() : mMap(32, 32) {
		mMap.AllocImage();
	}
	void			SaveAll();
};

