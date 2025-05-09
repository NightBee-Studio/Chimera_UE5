
#include "TsBiome.h"


//�e�N�X�`�����Z�[�u����

#if 0

#if WITH_EDITOR

#include "Materials/MaterialInstanceConstant.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "EditorAssetLibrary.

UMaterialInstanceConstant* CreateMaterialInstance(const FString& asset_path, UMaterial* parent_material)
{
	if (!parent_material) return nullptr;

	// Factory ���g���ăC���X�^���X���쐬
	UMaterialInstanceConstantFactoryNew* factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	factory->InitialParent = parent_material;

	// �p�X����́i��F"/Game/MyMaterials/NewMaterialInstance"�j
	FString p_name = asset_path;
	FString a_name;
	asset_path.Split(TEXT("/"), nullptr, &a_name, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	UPackage* package = CreatePackage(*p_name);
	UObject* asset = factory->FactoryCreateNew(UMaterialInstanceConstant::StaticClass(), package, FName(*a_name), RF_Public | RF_Standalone, nullptr, GWarn);

	// �ۑ����ăA�Z�b�g��
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