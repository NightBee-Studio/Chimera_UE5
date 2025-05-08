#include "TsBiomeSurface.h"
#include "TsBiome.h"
#include "TsBiomeMap.h"



// -------------------------------- Operators --------------------------------
#define	DECLARE
#include "TsOps.h"
#undef	DECLARE



// -------------------------------- Surfaces  --------------------------------

//class SurfaceWilderness : public TsBiomeSrfFunc {
//public:
//	SurfaceWilderness(const NoiseConfig& cnf)
//		: TsBiomeSrfFunc(cnf, 0.1f) {}
//};



// -------------------------------- TsBiomeSurface  --------------------------------

TsImageMap<float>* TsBiomeSurface::CreateMask(int reso, const FBox2D& bound)
{
	if (mMask != nullptr) delete mMask;
	mMask = new TsImageMap<float>( reso, reso, &bound );
	return mMask;
}

void			TsBiomeSurface::UpdateRemap(const FVector2D& p)
{
	for (auto s : mSurfaceFuncs) s->UpdateRemap(p);
	//for (auto s : mMaterialFuncs) s->UpdateRemap(p);
}

float			TsBiomeSurface::GetHeight(const FVector2D& p)
{
	float h = 0;
	for (auto s : mSurfaceFuncs) {
		h += s->Remap(s->GetValue(p));
	}
	return h;
}

TsMaterialValue	TsBiomeSurface::GetMaterial(const FVector2D& p)
{
	TsMaterialValue mv;
	for (auto fn : mMaterialFuncs) {
		mv.Merge(fn->GetMaterial(p));
	}
	return mv;
}

void	TsBiomeSurface::GatherBiome(TsBiome* b)
{
	bool done = false;
	for (TsBiomeGroup& gp : mGroups) {
		if (mGroupNum < 0 || gp.Num() < mGroupNum) {
			for (TsBiome* bm : gp) {
				for (auto& ed : bm->mEdges) {
					if (ed.mShared == b) {
						gp.Add(b);
						done = true;
						break;
					}
				}
				if (done) break;
			}
		}
		if (done) break;
	}
	if (!done) mGroups.Add(TsBiomeGroup{ b });
}




TArray<TsBiomeMatFunc*>	TsBiomeMatFunc::gList;

void	TsBiomeMatFunc::UpdateOccupancy()
{
	for (auto& fn : gList) {
		for (auto& c : fn->mConfigs) {
			TsBiomeMap* bm = TsBiomeMap::GetBiomeMap(c.mMapSource);
			TArray<float> img(bm->GetImage(), bm->GetW() * bm->GetH());
			img.Sort();
			float occupancy = 0;
			float min = img[0];
			for (auto& l : c.mLayers) {
				l.mMin = min;
				l.mMax = min = img[(img.Num() - 1) * FMath::Min(occupancy += l.mOccupancy, 1)];
			}
		}

		for (auto& c : fn->mConfigs) {
			for (auto& l : c.mLayers) {
				UE_LOG(LogTemp, Log, TEXT("GetMaterial [%d] Occ%f Range[%f %f]"), l.mMaterialType, l.mOccupancy, l.mMin, l.mMax);
			}
		}
	}
}


TsMaterialValue	TsBiomeMatFunc::GetMaterial(const FVector2D& p)
{
	TsMaterialValue mv;
	TsMaterialValue a, b;
	TArray<TsMaterialPixel>ppx;

	EMaterialType type = EMaterialType::MT_None;
	for (auto& c : mConfigs) {
		TsBiomeMap*		bm = TsBiomeMap::GetBiomeMap(c.mMapSource);
		float			v = bm->GetValue(p);
		TsMaterialValue	ml;
		for (auto& l : c.mLayers) {
			if (l.mMaterialType != EMaterialType::MT_None) {
				if (l.mMin <= v && v < l.mMax) {
					TsOp::gResultDone = false;
					if (l.mOp && !l.mOp->Is(p)) continue;
					if (TsOp::gResultDone) {
						ml.Merge(TsOp::gMatResult);
					} else {
						float val;
						if (type) {
							val = FMath::Min( (v - l.mMin) / ((l.mMax - l.mMin) * 0.4f), 1);
							if (val < 1) mv.Add(TsMaterialPixel(type, 1));
						} else {
							val = 1;
						}
						ml.Add(TsMaterialPixel(l.mMaterialType, val));
					}
					break;
				}
			}
			type = l.mMaterialType;
		}
		mv.Merge(ml);
	}

	//	UE_LOG(LogTemp, Log, TEXT("NoResult (%f %f) %d"), p.X, p.Y, mConfigs.Num());

	return mv;
}





void	TsBiomeSurface::ForeachGroup() {
	for (auto bg : mGroups) {
		for (auto fn : mSurfaceFuncs) {
			fn->Exec_EachGroup(bg);
		}
	}
}

