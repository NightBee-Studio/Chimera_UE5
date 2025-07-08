
#include "TsBiome.h"

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