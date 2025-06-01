#include "TsBiomeSurface.h"
#include "TsBiome.h"
#include "TsBiomeMap.h"



// -------------------------------- Operators --------------------------------
#define	DECLARE
#include "TsOps.h"
#undef	DECLARE



// -------------------------------- Surfaces  --------------------------------


// -------------------------------- TsBiomeSFunc --------------------------------

void	TsBiomeSFunc::RemapHeight(TsBiome* b, const FVector2D& p)
{
	float v = GetHeight( b, p );
	mMin = FMath::Min(mMin, v);
	mMax = FMath::Max(mMax, v);
}

// -------------------------------- TsBiomeMFunc --------------------------------

#if 0
TsMaterialPixel	TsBiomeMFunc::GetMaterial(const FVector2D& p)
{
	EMaterialType type = EMaterialType::MT_None;

	TsBiomeMap* bm = TsBiomeMap::GetBiomeMap(mMapType);
	float		v  = bm->GetValue(p);
	TsMaterialPixel	pl;
	for (const auto& l : mLayers) {
		if (l.mMaterialType != EMaterialType::MT_None) {
			if (l.mMin <= v && v < l.mMax) {
				float val;
				if ( type ) {
					val = FMath::Min((v - l.mMin) / ((l.mMax - l.mMin) * 0.4f), 1);
					//if (val < 1) px.Add( TsMaterialValue(type, 1) );
				} else {
					val = 1;
				}
				pl.Add( TsMaterialValue(l.mMaterialType, val) );
				break;
			}
		}
		type = l.mMaterialType;
	}
	//	UE_LOG(LogTemp, Log, TEXT("NoResult (%f %f) %d"), p.X, p.Y, mConfigs.Num());

	return pl;
}
#endif


// -------------------------------- TsBiomeSurface  --------------------------------

void			TsBiomeSurface::UpdateRemap(TsBiome* b, const FVector2D& p)
{
	for (auto s : mSFuncs) {
		s->RemapHeight(b, p);
	}
	//for (auto s : mMaterialFuncs) s->UpdateRemap(p);
}

float			TsBiomeSurface::GetHeight(TsBiome* b, const FVector2D& p)
{
	float h = 0;
	for (auto s : mSFuncs) {
		h += s->Remap(s->GetHeight(b, p));
	}
	return h;
}

//TsMaterialPixel	TsBiomeSurface::GetMaterial(TsBiome* b, const FVector2D& p)
//{
//	TsMaterialPixel px;
//	for (auto fn : mMFuncs) {
//		px.Merge(fn->GetMaterial(p));
//	}
//	return px;
//}

//TArray<TsBiomeMatFunc*>	TsBiomeMatFunc::gList;
//
//void	TsBiomeMatFunc::UpdateOccupancy()
//{
//	for (auto& fn : gList) {
//		for (auto& c : fn->mConfigs) {
//			TsBiomeMap* bm = TsBiomeMap::GetBiomeMap(c.mMapSource);
//			TArray<float> img(bm->GetImage(), bm->GetW() * bm->GetH());
//			img.Sort();
//			float occupancy = 0;
//			float min = img[0];
//			for (auto& l : c.mLayers) {
//				l.mMin = min;
//				l.mMax = min = img[(img.Num() - 1) * FMath::Min(occupancy += l.mOccupancy, 1)];
//			}
//		}
//
//		for (auto& c : fn->mConfigs) {
//			for (auto& l : c.mLayers) {
//				UE_LOG(LogTemp, Log, TEXT("GetMaterial [%d] Occ%f Range[%f %f]"), l.mMaterialType, l.mOccupancy, l.mMin, l.mMax);
//			}
//		}
//	}
//}

