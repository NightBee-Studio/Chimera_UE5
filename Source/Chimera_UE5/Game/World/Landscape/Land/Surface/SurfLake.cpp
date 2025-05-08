#include "SurfLake.h"
#include "../../Util/TsUtility.h"







struct LakeShape {
	TArray<TsVoronoi::Edge*>		mEdges;
	TArray<TsBiome*>				mGroup;

	static TArray<LakeShape*>	gShapes;


	static LakeShape* Create(TArray<TsBiome*>& list)
	{
		LakeShape* shape = new LakeShape();
		shape->mGroup = list;
		for (auto b : list) {
			for (auto& ed : b->mEdges) {
				TsBiome* bm = (TsBiome*)ed.mShared;
				if (bm && list.Find(bm) != INDEX_NONE) continue;
				shape->mEdges.Add(&ed);
			}
		}
		return shape;
	}

	float	GetValue(const FVector2D& p) {
		for (auto b : mGroup) {
			if (b->IsInside(p)) {
				float h = 100000.0f;
				for (auto e : mEdges) {
#if 0
#define SL	5.0f
#define JL	8.0f
					FVector2D	step = e->mD;
					int			n = (int)(step.Length() / SL) + 1;
					for (int i = 0; i < n; i++) {
						auto jitter = [&](const FVector2D& pp) {
#define JS	0.015f
							return	pp + JL * FVector2D(
								FMath::PerlinNoise1D(pp.X * JS + 205.0f),
								FMath::PerlinNoise1D(pp.Y * JS - 652.0f));
						};
						FVector2D	v0 = jitter(e->mP + step * (i + 0) / n);
						FVector2D	v1 = jitter(e->mP + step * (i + 1) / n);
						h = FMath::Min(h, (near_point(v0, v1, p) - p).Length());
					}
#endif
					float hc = (near_point(e->mP, e->mP + e->mD, p) - p).Length();
					h = FMath::Min(hc, h);
				}
				return h / 50;
			}
		}
		return 0;
	}
};

TArray<LakeShape*>	LakeShape::gShapes;



float	SurfaceLake::GetValue(const FVector2D& p) 
{
	float h = 0;
	for (auto& shape : LakeShape::gShapes) {
		h = FMath::Max(h, shape->GetValue(p));
	}
	return h;
}

float	SurfaceLake::Remap(float val) const
{
	return mHeight * FMath::Pow(TsNoiseMap::Remap(val), 0.5f );
}

void	SurfaceLake::Exec_EachGroup(TsBiomeGroup& grp) {
	LakeShape::gShapes.Add( LakeShape::Create(grp) );
}
