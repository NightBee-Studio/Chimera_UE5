#include "TsSurfMountain.h"
#include "TsSurfUtility.h"

#include "../TsBiome.h"



struct Mountain {
	TArray<TsBiome*>	mBiomes ;
	float				mMin, mMax ;

	Mountain() : mMin(10000),mMax(-10000) {}

	bool	IsInside( const FVector2D& p ) const {
		for ( auto b : mBiomes ){
			if ( b->IsInside( p ) ) return true;
		}
		return false;
	}
};


class Mountain_Work {
public:
	TArray<Mountain>	mMountains ;
	TArray<TsBiome*>	mDoneList ;

	bool	TryDone( TsBiome* b ) {
//		UE_LOG( LogTemp, Log, TEXT("TryDone[%p] %d==%d"), b, mDoneList.Find(b), INDEX_NONE );
		if ( mDoneList.Find(b)==INDEX_NONE ) {	mDoneList.Add( b ); return false ;}
		return true ;
	}

	Mountain_Work( TArray<TsBiome> & biome_list){
		for ( auto &b : biome_list ){
			if ( b.mSType == EBiomeSType::EBSf_Mountain ){
				TsBiome * bb = &b ;
				if ( !TryDone( bb ) ){
					Mountain m ;

					TArray<TsBiome*>query = { bb } ;
					while ( query.Num() > 0 ){		// Gather the adjacent biome.
						m.mBiomes.Add( bb = query[0] ) ;
						query.Remove( bb ) ;

						for ( auto ed : bb->mEdges ){
							if ( ed.mShared ) {
								TsBiome *a = (TsBiome *)ed.mShared->mOwner ;
								if ( a->mSType==bb->mSType ){
									if ( !TryDone( a ) ) query.Add( a ) ;
								}
							}
						}
					}

					UE_LOG( LogTemp, Log, TEXT(" Mountain BiomeNum (%f,%f) %d"), m.mBiomes[0]->X, m.mBiomes[0]->Y, m.mBiomes.Num() );

					mMountains.Add( m ) ;
				}
			}
		}
	}

	void UpdateMountain( const FVector2D &pos, float h ){
		for ( auto &m : mMountains ){
			if ( m.IsInside(pos)){
				m.mMin = FMath::Min( m.mMin, h );
				m.mMax = FMath::Max( m.mMax, h );
			}
		}
	}

} ;

void * TsSurfaceMountain::gWork ;

void TsSurfaceMountain::Initialize( TArray<TsBiome>	& biome_list )
{
	gWork = new Mountain_Work( biome_list ) ;
}

void TsSurfaceMountain::UpdateMountain( const FVector2D &pos, float h )
{
	((Mountain_Work*)gWork)->UpdateMountain( pos, h ) ;
}










// -------------------------------- SurfaceMountain --------------------------------

float	TsSurfaceMountain::GetHeight(TsBiome* b, const FVector2D& p)
{
	TArray<TsBiome*> done_list;
	float h = TsSurfUtil::RecurseGetHeight( b, p, done_list, 1.0f );
	UpdateMountain( p, h ) ;
	return h ;
}

float	TsSurfaceMountain::Remap(float val) const
{
	return mHeight * FMath::Pow(TsNoiseMap::Remap(val), mPowerFactor);
}


