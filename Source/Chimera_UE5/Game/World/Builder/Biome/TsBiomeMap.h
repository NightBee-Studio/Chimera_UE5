#pragma once

#include "CoreMinimal.h"

#include "TsBiome.h"
#include "../Util/TsImageMap.h"
#include "../Util/TsBuilderTool.h"



// -------------------------------- TsBiomeMaps  --------------------------------

//
//enum EBiomeMapType {
//	E_None,	//nothing
//
//	E_Moist,	//moisture
//	E_Tempr,	//temperture
//	E_Genre,	//genre
//	E_Flow,	//flow
//	E_Pond,	//Pond
//
//	E_Mountain,	//
//	E_Slope,	//
//	E_Plant,	//bigger = taller
//
//	E_Normal,	//
//};





//	Hue Sat Val		=>	 	Genre Temp Moist
class TsBiomeMap
	: public TsLevelMap
	, public TsNoiseMap {
private:
	static
	TMap<ETextureMap, TsBiomeMap*>	gBiomeMaps;
public:
	static TsBiomeMap*	GetBiomeMap(ETextureMap ty	             ) { return gBiomeMaps[ty]; }
	static void			AddBiomeMap(ETextureMap ty, TsBiomeMap* map) { gBiomeMaps.Emplace( ty, map ); }

public:
	TsBiomeMap(int w, int h, const FBox2D* bound=nullptr, const TsNoiseParam& cnf = TsNoiseParam())
		: TsLevelMap(w, h, bound)
		, TsNoiseMap(cnf) {}
	TsBiomeMap(UTexture2D *tex, const FBox2D* bound=nullptr, const TsNoiseParam& cnf = TsNoiseParam())
		: TsLevelMap(tex, bound)
		, TsNoiseMap(cnf) {}
	virtual ~TsBiomeMap() {}

	virtual float	RemapImage(float v, float range) const override;
	virtual float	GetValue(const FVector2D& p) override;
	virtual void	SetPixel(int x, int y, float v)override;

	// Items
public:
	template<
		DerivedFVector2D	T_point,
		DerivedBiomeItem	T_item
	>
	void SetupItems( TArray<T_point>& points, TArray<T_item>& items ) {
		TArray<float>	samples;
		for (const auto& p : points) samples.Add( TsBiomeMap::GetValue(p) );
		samples.Sort();		// sort to determine the ratio of the group.

		int max_idx = (samples.Num() - 1);
		float			ratio = 0.0f;
		for (auto& it : items) {
			ratio += it.mRatio;
			it.mThreshold = samples[max_idx * FMath::Min(1, ratio)];
		}
	}

	template<
		DerivedBiomeItem T_item
	>
	void SetupItemsPixel( TArray<T_item>& items, std::function< bool(int, int) > chk_pixel = nullptr, bool valueskip = true ) {
		TArray<float>	samples;
		ForeachPixel(
			[&](int px, int py) {
				if (chk_pixel ? chk_pixel(px,py) : true ){
					float val = TsBiomeMap::GetValue( GetWorldPos(px, py) ) ;
					if ( valueskip ? val!=0.0f : true ) samples.Add( val );
				}
			});
		samples.Sort();		// sort to determine the ratio of the group.

		int    max_idx = (samples.Num() - 1);
		float  ratio = 0.0f;
		for (auto& it : items) {
			ratio += it.mRatio;
			it.mThreshold = samples[max_idx * FMath::Min(1, ratio)];
		}
	}

	template<
		DerivedFVector2D T_point,
		DerivedBiomeItem T_item
	>
	int SelectItemIdx( const T_point& point, TArray<T_item>& items ) {
		float h = GetValue(point);
		for (int i = 0; i < items.Num(); i++) {
			if (h < items[i].mThreshold) return i;
		}
		return items.Num() - 1;
	}
	template<
		DerivedFVector2D T_point,
		DerivedBiomeItem T_item
	>
	float SelectItemValue( const T_point& point, TArray<T_item>& items ) {
		float h = GetValue(point);
		for (int i = 0; i < items.Num(); i++) {
			if (h < items[i].mThreshold){
				float min = (i>0 ? items[i-1].mThreshold : mMin) ;
				float max = items[i].mThreshold ;
				return (h-min)/(max-min) ;
			}
		}
		return 0 ;
	}

	template<
		DerivedFVector2D T_point,
		DerivedBiomeItem T_item
	>
	T_item SelectItem( const T_point& point, TArray<T_item>& items ) {
		return items[SelectItemIdx(point, items)];
	}
};


class TsHeightMap
	: public TsBiomeMap 
{
public:
	TsHeightMap( int w, int h, const FBox2D* bound)
		: TsBiomeMap(w, h, bound) {}
	virtual ~TsHeightMap() {}

	FVector CalcNormal(
		float x, float y,// texel coordinate
		float grid_height = 1.0f,
		float grid_size = 1.0f);

	void	Normalize();

	float	RemapImage(float v, float range ) const override;
};


enum EExtraOp{
	E_Add,
	E_Mul,
	E_InvMul,
} ;

struct TsExtraMap 
: public TsBiomeMap {
	float		mScale;
	EExtraOp	mOp;
	TsExtraMap( ETextureMap texkey, UTexture2D *tex, const FBox2D* bound, float scale, EExtraOp op )
		: TsBiomeMap(tex, bound), mScale( scale), mOp( op ){
	//	TsBiomeMap::AddBiomeMap( texkey, this ) ;
	}
} ;

class TsMoistureMap
	: public TsBiomeMap
{
public:
	TsMoistureMap(
			const FBox2D*						bound,
			const TsNoiseParam&					cnf, 
			std::initializer_list<TsExtraMap>	extra
		)
		: TsBiomeMap( extra.begin()->GetW(), extra.begin()->GetH(), bound, cnf )
		, mExtras( extra ){}

	TArray<TsExtraMap>	mExtras ;

	void Lock() ;
	void UnLock() ;

	virtual float	GetValue(const FVector2D& p) override;
};




struct TsMaterialPixel
{
	TMap<EMaterialType, float>	mValues;

	void Add(EMaterialType t, float v) {
		if (mValues.Contains(t))	mValues[t] += v;
		else						mValues.Emplace( t, v );
	}
	void Max( EMaterialType t, float v){
		if ( mValues.Contains(t) )	mValues[t] = FMath::Max(v, mValues[t]);
		else						mValues.Emplace( t, v );
	}

	void Clear() {
		mValues.Empty() ;
	}

	void Normalize() {
		float total = 0;
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
	TArray<TsGroundSlot>	mTexParams;

public:
	TsMaterialMap(int w, int h, const FBox2D* bound );
	virtual ~TsMaterialMap() {}


	void				MergePixel(int x, int y, const TsMaterialPixel& px );
	void				SetPixel(int x, int y, EMaterialType ty, float val);
	TsMaterialPixel&	GetPixel(int x, int y) ;
	const TArray<TsGroundSlot>&	GetTexParams()	{ return mTexParams ;}

	void				ForeachPix(std::function< void(int, int, TsMaterialPixel&) >, int inc = 1);

	void				Clear();
	void				StoreMaterial();

	void				SaveAll(int x, int y, int w, int h);
};
