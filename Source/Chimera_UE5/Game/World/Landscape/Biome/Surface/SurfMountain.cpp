#include "SurfMountain.h"
#include "../../Util/TsUtility.h"




#if 0

struct MtShape : public TsNoiseMap {
	static TArray<MtShape*>	gShapes;

	virtual
	bool	IsInside(const FVector2D& p) { return false; }

	virtual 
	float	GetValue( const FVector2D& p) { return 0; }

	virtual
	void	GetMountains(TArray<TsVoronoi>& v_list){}

};


struct MtShapeB : public MtShape {
	TArray<TsVoronoi::Edge*>	mEdges;
	TArray<TsBiome*>			mGroup;
	float						mGradient;

	virtual ~MtShapeB() {}

	void	GetMountains(TArray<TsVoronoi>& v_list) override {
		for (auto bm : mGroup ) {
			v_list.Add( *bm );
		}
	}

	static MtShape* Create(TArray<TsBiome*>& list)
	{
		FBox2D	boundingbox(FVector2D(10000, 10000), FVector2D(-10000, -10000));

		MtShapeB* shape  = new MtShapeB();
		shape->mGroup    = list;
		for (auto b : list) {
			boundingbox += *b;

			//b->ForeachEdge([&shape,list]( const TsVoronoi::Edge& e ) {
			//		TsBiome* bm = (TsBiome*)e.mShared;
			//		if ( bm && list.Find(bm) != INDEX_NONE) continue;
			//		shape->mEdges.Add(e);
			//	});

			for ( auto& ed : b->mEdges){
				TsBiome* bm = (TsBiome*)ed.mShared;
				if (bm && list.Find(bm) != INDEX_NONE) continue;
				shape->mEdges.Add(&ed);
			}
		}
		FVector2D size = boundingbox.GetSize();
		shape->mGradient = FMath::Min(size.X, size.Y) < 300 ? 50.0f : 100.0f ;
		UE_LOG(LogTemp, Log, TEXT("MtShapeB:: size %f %f"), boundingbox.GetSize().X, boundingbox.GetSize().Y);

		return shape;
	}

	bool IsInside(const FVector2D& p) override {
		for (auto b : mGroup) {
			if (b->IsInside(p)) return true;
		}
		return false;
	}

	float	GetValue(const FVector2D& p) override {
		for (auto b : mGroup) {
			if (b->IsInside(p)) {
				float h = 100000.0f;

				b->ForeachEdge([&h, p ](const TsVoronoi::Edge& e) {
						float hc = (TsUtil::NearPoint(e.mP, e.mP + e.mD, p) - p).Length();
						h = FMath::Min(hc, h);
					});

				//for (auto e : mEdges) {
				//	float hc = (TsUtil::NearPoint(e->mP, e->mP + e->mD, p) - p).Length();
				//	h = FMath::Min(hc, h);
				//}
				return h / mGradient ;
			}
		}
		return 0;
	}
};

TArray<MtShape*>	MtShape::gShapes;

#endif



// -------------------------------- SurfaceMountain --------------------------------

#if 0
SurfaceMountain*	SurfaceMountain::gInstance = nullptr;
TsNoiseMap*			SurfaceMountain::gNoiseMap = nullptr;

void	SurfaceMountain::GetMountains(TArray<TsVoronoi>& v_list)
{
	for (auto m : MtShape::gShapes) {
		m->GetMountains(v_list);
	}
}

float	SurfaceMountain::GetMountValue(const FVector2D& p)
{
	for (auto& shape : MtShape::gShapes) {
		if (shape->IsInside(p)) {
			return 
				shape->Remap(shape->GetValue(p)) + gNoiseMap->Remap(gNoiseMap->GetValue(p)) * 0.3f;
		}
	}
	return 0;
}
#endif


//float	SurfaceMountain::GetValue(const FVector2D& p) 
//{
//	float h = 0;
//	for (auto &shape : MtShape::gShapes) {
//		h = FMath::Max( h, shape->GetValue(p) );
//	}
//	return h;
//}

float	SurfaceMountain::Remap(float val) const
{
	return mHeight * FMath::Pow(TsNoiseMap::Remap(val), mPowerFactor );
}


//void	SurfaceMountain::Exec_UpdateRemap(const FVector2D& p)
//{
////	gNoiseMap->UpdateRemap(p);
//	for (auto& shape : MtShape::gShapes) {
//		shape->UpdateRemap(p);
//	}
//}
//
//void	SurfaceMountain::Exec_EachGroup( TsBiomeGroup& grp)
//{
////	CreateMountainB (grp );
//}

float	SurfaceMountain::GetHeight(TsBiome* b, const FVector2D& p)
{
	float h = 0;
	b->ForeachEdge([&h, p](const TsVoronoi::Edge& e) {
			float hc = (TsUtil::NearPoint(e.mP, e.mP + e.mD, p) - p).Length();
			h = FMath::Max(hc, h);
		});
	UE_LOG(LogTemp, Log, TEXT("Mnt[%d,%d] %f"), (int)p.X, (int)p.Y, h);

	UpdateRemap( h );

	return h;
}

//
//void	SurfaceMountain::CreateMountainA(FVector2D pos, float radius, int branch_max )
//{
//	//MtShape::gShapes.Add( MtShapeA::Create( pos, radius, branch_max ) );
//}
//
//void	SurfaceMountain::CreateMountainB(TArray<TsBiome*>& list )
//{
//	MtShape::gShapes.Add( MtShapeB::Create(list) );
//}
//
//void SurfaceMountain::Debug( UWorld *world ){
//	for (auto s : MtShape::gShapes) {
////		s->mKnots->DebugMountShape( world );
//	}
//}
//

