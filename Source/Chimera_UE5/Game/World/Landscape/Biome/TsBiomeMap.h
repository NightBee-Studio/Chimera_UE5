#pragma once

#include "CoreMinimal.h"

#include "TsBiome.h"
#include "../Util/TsImageMap.h"



// -------------------------------- TsBiomeMaps  --------------------------------


enum EBiomeMapType {
	E_None,	//nothing

	E_Moist,	//moisture
	E_Tempr,	//temperture
	E_Genre,	//genre
	E_Flow,	//flow
	E_Pond,	//Pond

	E_Mountain,	//
	E_Slope,	//
	E_Plant,	//bigger = taller

	E_Normal,	//
};



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
	template<DerivedFromTsFVector2D Tpoint, DerivedFromTsBiomeItem Titem>
	void SetupItems(TArray<Tpoint>& points, TArray<Titem>& items) {
		TArray<float>	samples;
		float			ratio = 0.0f;
		for (const auto& p : points) samples.Add(GetValue(p));
		samples.Sort();		// sort to determine the ratio of the group.
		int max_idx = (samples.Num() - 1);
		for (auto& it : items) {
			ratio += it.mRatio;
			it.mThreshold = samples[max_idx * FMath::Min(1, ratio)];
		}
	}

	template<DerivedFromTsFVector2D Tpoint, DerivedFromTsBiomeItem Titem>
	Titem SelectItem(const Tpoint& point, TArray<Titem>& items ) {
		float h = GetValue(point);
		for (auto& it : items) {
			if (h < it.mThreshold) {
				return it ;
			}
		}
		return items.Last() ;
	}
};


class TsHeightMap
	: public TsBiomeMap 
{
public:
	TsHeightMap(int w, int h, const FBox2D* bound) : TsBiomeMap(w, h, bound) {}
	virtual ~TsHeightMap() {}

	FVector CalcNormal(
		float x, float y,// texel coordinate
		float grid_height = 1.0f,
		float grid_size = 1.0f);

	void	Normalize();

	float	RemapImage(float v, float range ) const override;
};

class TsNormalMap
	: public TsImageMap<FVector>
{
public:
	TsNormalMap(int w, int h, const FBox2D* bound) : TsImageMap<FVector>(w, h, bound) {}
};





struct TsMaterialPixel
{
	TMap<EMaterialType, float>	mValues;

	void Add( EMaterialType t, float v){
		if ( mValues.Contains(t) )	mValues[t] += v ;
		else						mValues.Emplace( t, v );
	}

	void Normalize() {
		float total = 0;
		for (auto& v : mValues) 
			UE_LOG(LogTemp, Log, TEXT("    [[ty%d w%f]]"), v.Key, v.Value);

		for ( auto& v : mValues) total += v.Value ;
		for ( auto& v : mValues) v.Value /= total ;
	}

	void Merge(const TsMaterialPixel& pixel, float rate = 1) {
		for ( auto &v : pixel.mValues ) {
			Add(v.Key, v.Value * rate);
		}
	}
};


class TsMaterialMap
	:public TsImageCore
{
private:
	TArray<TsMaterialPixel>	mPixels;
	TsImageMap<int>			mIndexMap;	//RGBA
	TsImageMap<int>			mAlphaMap;	//RGBA
	TArray<EMaterialType>	mMatIndex;

public:
	TsMaterialMap(int w, int h, const FBox2D* bound, const TArray<EMaterialType> &mat_index  );
	virtual ~TsMaterialMap() {}

	void				MergePixel(int x, int y, const TsMaterialPixel& px );
	void				SetPixel(int x, int y, EMaterialType ty, float val);
	TsMaterialPixel&	GetPixel(int x, int y) ;

	void				StoreMaterial();

	void				SaveAll(int x, int y, int w, int h);
};
