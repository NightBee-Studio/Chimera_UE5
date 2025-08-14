
#include "TsBiome.h"

#include "TsBiomeModel.h"

#include "../Util/TsUtility.h"



TsBiome::TsBiome(float x, float y)
	: TsVoronoi(x, y, this)
	, mSType( EBiomeSType::EBSf_None )
	, mMType( EBiomeMType::EBMo_Soil )
{
}

float	TsBiome::GetMask(const FVector2D& p)
{
	float msk = 0 ;

	if ( IsInside( p ) ){
		msk = 1.0f ;
		ForeachEdge(
			[&](const TsVoronoi::Edge& e) {
				float m = FMath::Clamp(e.GetDistance(p) / 100, 0.0f, 1.0f);
				if ( e.mShared && e.mShared->mOwner ) {		// check the adjecent voronois.
					TsBiome* nxt = (TsBiome*)e.mShared->mOwner;
					if (nxt->GetSType() >= GetSType()) m = msk;
				}
				msk = FMath::Min(m, msk);
			} );
	}
	return msk;
}


void TsBiome::GetBlend(TMap<TsBiome*,float>& blend_map, const FVector2D& p)
{
	blend_map.Empty();

	ForeachEdge(
		[&](const TsVoronoi::Edge& e) {
			if (e.mShared && e.mShared->mOwner) {		// check the adjecent voronois.
				float d = FMath::Pow(e.GetDistance(p)/100, 0.5f);
				float r = FMath::Clamp(d, 0.0f, 1.0f);
				if (r < 1.0f) {
					TsBiome* nxt = (TsBiome*)e.mShared->mOwner;
					if (nxt->GetSType() < GetSType()) {
						blend_map.Emplace( nxt, 1-r );
					}
				}
			}
		});
	//UE_LOG(LogTemp, Log, TEXT("BLMap[%d] --------------------"), blend_map.Num() );
}

//テクスチャをセーブする
static TArray<TsBiome*>	gDoneList ;
static int				gSeqNo = 0 ;

bool TsBiomeGroup::CheckDone( TsBiome* b )
{
	//UE_LOG(LogTemp, Log, TEXT("   CheckDone %d==%d"), gDoneList.Find(b), INDEX_NONE );

	return gDoneList.Find(b) != INDEX_NONE ;
}

bool TsBiomeGroup::TryDone( TsBiome* b ) {
	bool done = CheckDone(b) ;
	if ( !done ) Done( b ) ;
	return done ;
}

void TsBiomeGroup::Done( TsBiome* b )
{
	gDoneList.Add( b );
}

void TsBiomeGroup::ClearDone() {
	gDoneList.Empty();
}


TsBiomeGroup::TsBiomeGroup( TsBiome* b, FTsBiomeModels& biome_models )
{
	int count = 
		b->mSType==EBiomeSType::EBSf_Mountain ? 0  ://3
		b->mSType==EBiomeSType::EBSf_Field    ? 0  :
		b->mSType==EBiomeSType::EBSf_Lake     ? 15 :
		0 ;

	mID	= FTsBiomeSpec::ID(b->mGType, b->mSType);

//	FRotator			rot( 0, 0, TsUtil::RandRange(-180,180) ) ;
	FRotator			rot( 0, 0, 0 ) ;
	FVector				pos( 0, 0, 0 ) ;
	float				scale = 1.0f ;
	TArray<TsBiome*>	query = { b } ;

	auto update_biomes = [&]( TsBiome *b ){
		mBiomes.Add( b ) ;
		pos += FVector( b->X, b->Y, 0 ) ;
		mMin = FVector2D( FMath::Min(b->X, mMin.X), FMath::Min(b->Y, mMin.Y));
		mMax = FVector2D( FMath::Max(b->X, mMax.X), FMath::Max(b->Y, mMax.Y));
	};

	// Gather the adjacent biome.
	while ( query.Num() > 0 ){
		update_biomes( b = query[0] ) ;
		query.Remove( b ) ;

		UE_LOG(LogTemp, Log, TEXT(" (%f,%f) Type%d  query%d/count%d"),  b->X, b->Y, b->mSType, query.Num(), count );

		for ( auto ed : b->mEdges ){
			if ( count <= 0 ) break ;
			if ( ed.mShared ) {
				TsBiome *a = (TsBiome *)ed.mShared->mOwner ;
				if ( a->mSType==b->mSType ){
					if ( !TryDone( a ) ){
						query.Add( a ) ;
						count-- ;
					//	UE_LOG(LogTemp, Log, TEXT("   Add (%f %f)   qurey%d/count%d"), a->X, a->Y,  query.Num(), count  );
					}
				}
			}
		}
	}
	//mSeqID		 = ++gSeqNo ;
	mModel		 = biome_models.GetRandomModel( mID ) ;
	mHeightRange = biome_models.GetHeightRange( mID ) ;
	FVector2D size = mMax - mMin ;
	scale		 = mModel->GetSizeX() / (size.X>size.Y ? size.X : size.Y) ;
	pos			/= mBiomes.Num() ;
	mTransform	 = FTransform( rot, pos, FVector(scale) ) ;
	UE_LOG(LogTemp, Log, TEXT("  G(%f,%f) id%d BNum=%d  size(%f %f) Reso%d"), pos.X, pos.Y, mID, mBiomes.Num(), size.X, size.Y, mModel->GetSizeX() );
}


bool	TsBiomeGroup::IsInside( const FVector2D& p ) const
{
	for (auto b : mBiomes){
		if ( b->IsInside(p) ) return true;
	}
	return false;
}

float	TsBiomeGroup::GetMask(const FVector2D& p) const
{
	return 0.0f ;
}

float	TsBiomeGroup::GetPixel(ETextureMap mp, const FVector2D& p) const
{
	if ( mModel && mModel->mTextureMaps.Contains(mp) ) {
		FVector local = mTransform.InverseTransformPosition( FVector( p.X, p.Y, 0 ));

		float h = mModel->mTextureMaps[mp]->GetValue( FVector2D(local.X, local.Y) ) ;
		UE_LOG(LogTemp, Log, TEXT("(%f %f)->(%f %f)  Transf(%f %f) h=%f"), p.X, p.Y, local.X, local.Y, mTransform.GetLocation().X, mTransform.GetLocation().Y, h );

		return mHeightRange.X + (mHeightRange.Y - mHeightRange.X) * h ;
	}
	return 0 ;
}
