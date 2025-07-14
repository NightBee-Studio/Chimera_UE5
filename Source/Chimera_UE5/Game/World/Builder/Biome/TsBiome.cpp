
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

//	TMap<EBiomeSType, float > blend_map;
	//float bmax = 0.0f;
	//float bsum = 0.0f;
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

	//for (auto& bl : blend_map) {
	//	UE_LOG(LogTemp, Log, TEXT("  [%d]%f"), bl.Key->GetSType(), bl.Value);
	//}
	
	//for (auto& bl : blend_array) {
	//	bl.mRatio = bmax * bl.mRatio / bsum;
	//}
	//blend_array.Add({ this, 1.0f - bmax });
}


//テクスチャをセーブする

#if 0

#if WITH_EDITOR

#include "Materials/MaterialInstanceConstant.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include <winnt.h>
#include "EditorAssetLibrary.

UMaterialInstanceConstant* CreateMaterialInstance(const FString& asset_path, UMaterial* parent_material)
{
	if (!parent_material) return nullptr;

	// Factory を使ってインスタンスを作成
	UMaterialInstanceConstantFactoryNew* factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	factory->InitialParent = parent_material;

	// パスを解析（例："/Game/MyMaterials/NewMaterialInstance"）
	FString p_name = asset_path;
	FString a_name;
	asset_path.Split(TEXT("/"), nullptr, &a_name, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	UPackage* package = CreatePackage(*p_name);
	UObject* asset = factory->FactoryCreateNew(UMaterialInstanceConstant::StaticClass(), package, FName(*a_name), RF_Public | RF_Standalone, nullptr, GWarn);

	// 保存してアセット化
	if ( asset ){
		FAssetRegistryModule::AssetCreated(asset);
		package->MarkPackageDirty();

		FString path = FPackageName::LongPackageNameToFilename(p_name, FPackageName::GetAssetPackageExtension());
		bool success = UPackage::SavePackage(package, asset, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *path);

		return Cast<UMaterialInstanceConstant>(asset);
	}

	return nullptr;
}
#endif		//WITH_EDITOR

#endif


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
	mSeqID		 = ++gSeqNo ;
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

		float h = mModel->mTextureMaps[mp]->GetValue( local.X, local.Y, EAnchor::E_CC ) ;
		UE_LOG(LogTemp, Log, TEXT("(%f %f)->(%f %f)  Transf(%f %f) h=%f"), p.X, p.Y, local.X, local.Y, mTransform.GetLocation().X, mTransform.GetLocation().Y, h );

		return mHeightRange.X + (mHeightRange.Y - mHeightRange.X) * h ;
	}
	return 0 ;
}
