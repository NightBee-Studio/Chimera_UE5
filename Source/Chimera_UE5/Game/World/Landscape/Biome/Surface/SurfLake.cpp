#include "SurfLake.h"
#include "../../Util/TsUtility.h"






//
//struct LakeShape {
//	TArray<TsVoronoi::Edge*>		mEdges;
//	TArray<TsBiome*>				mGroup;
//
//	static TArray<LakeShape*>	gShapes;
//
//
//	static LakeShape* Create(TArray<TsBiome*>& list)
//	{
//		LakeShape* shape = new LakeShape();
//		shape->mGroup = list;
//		for (auto b : list) {
//			for (auto& ed : b->mEdges) {
//				TsBiome* bm = (TsBiome*)ed.mShared;
//				if (bm && list.Find(bm) != INDEX_NONE) continue;
//				shape->mEdges.Add(&ed);
//			}
//		}
//		return shape;
//	}
//
//	float	GetValue(const FVector2D& p) {
//		for (auto b : mGroup) {
//			if (b->IsInside(p)) {
//				float h = 100000.0f;
//				for (auto e : mEdges) {
//					float hc = (TsUtil::NearPoint(e->mP, e->mP + e->mD, p) - p).Length();
//					h = FMath::Min(hc, h);
//				}
//				return h / 50;
//			}
//		}
//		return 0;
//	}
//};
//
//TArray<LakeShape*>	LakeShape::gShapes;


float	SurfaceLake::GetHeight(TsBiome* b, const FVector2D& p)
{
	float h = 0;
	if (b->IsInside(p)) {
		h = 100000.0f;
		b->ForeachEdge([&](const TsVoronoi::Edge& e) {
				float hc = (TsUtil::NearPoint(e.mP, e.mP + e.mD, p) - p).Length();
				h = FMath::Min(hc, h);
			}) ;
		h /= 50.0f;
	}
	UpdateRemap(h);
	return h;
}


//float	SurfaceLake::GetValue(const FVector2D& p) 
//{
//	float h = 0;
//	for (auto& shape : LakeShape::gShapes) {
//		h = FMath::Max(h, shape->GetValue(p));
//	}
//	return h;
//}

float	SurfaceLake::Remap(float val) const
{
	return mHeight * FMath::Pow(TsNoiseMap::Remap(val), 0.5f );
}

//void	SurfaceLake::Exec_EachGroup(TsBiomeGroup& grp) {
//	LakeShape::gShapes.Add( LakeShape::Create(grp) );
//}
//

