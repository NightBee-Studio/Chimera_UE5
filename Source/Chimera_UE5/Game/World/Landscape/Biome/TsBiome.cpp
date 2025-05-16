
#include "TsBiome.h"

#include "../Util/TsUtility.h"



TsBiome::TsBiome(float x, float y)
	: TsVoronoi(x, y, this)
	, mSType( EBiomeSType::E_SurfNone )
	, mMType( EBiomeMType::E_Soil     )
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
					if (nxt->GetSType() == GetSType()) m = msk;
				}
				msk = FMath::Min(m, msk);
			} );
	}
	return msk;
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