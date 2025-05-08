#include "SurfMountain.h"
#include "../../Util/TsUtility.h"






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
				for (auto e : mEdges) {
#if 0
#define SL	5.0f
#define JL	8.0f
					FVector2D	step = e->mD ;
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
				return h / mGradient ;
			}
		}
		return 0;
	}
};

TArray<MtShape*>	MtShape::gShapes;

// -------------------------------- SurfaceMountain --------------------------------

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

float	SurfaceMountain::GetValue(const FVector2D& p) 
{
	float h = 0;
	for (auto &shape : MtShape::gShapes) {
		h = FMath::Max( h, shape->GetValue(p) );
	}
	return h;
}

float	SurfaceMountain::Remap(float val) const
{
	return mHeight * FMath::Pow(TsNoiseMap::Remap(val), mPowerFactor );
}


void	SurfaceMountain::Exec_UpdateRemap(const FVector2D& p)
{
	gNoiseMap->UpdateRemap(p);
	for (auto& shape : MtShape::gShapes) {
		shape->UpdateRemap(p);
	}
}

void	SurfaceMountain::Exec_EachGroup( TsBiomeGroup& grp)
{
	CreateMountainB (grp );
}


void	SurfaceMountain::CreateMountainA(FVector2D pos, float radius, int branch_max )
{
	//MtShape::gShapes.Add( MtShapeA::Create( pos, radius, branch_max ) );
}

void	SurfaceMountain::CreateMountainB(TArray<TsBiome*>& list )
{
	MtShape::gShapes.Add( MtShapeB::Create(list) );
}

void SurfaceMountain::Debug( UWorld *world ){
	for (auto s : MtShape::gShapes) {
//		s->mKnots->DebugMountShape( world );
	}
}


