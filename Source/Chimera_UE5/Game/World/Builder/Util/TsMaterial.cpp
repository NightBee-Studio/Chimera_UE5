#include "TsMaterial.h"

#include "Materials/MaterialInstanceConstant.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

#include "TsUtility.h"


// Your original Build function rewritten to match UE5.4-style StaticMesh LOD/mesh description usage
UMaterialInstanceConstant* TsMaterial::Build(
        const FString& msmat_path,
        const FString& asset_name
    )
{
    UMaterialInstanceConstant* material = nullptr ;
#if WITH_EDITOR
    UMaterialInterface* master_mat = LoadObject<UMaterialInterface>(nullptr, *msmat_path);//TEXT("/Game/Materials/M_Master.M_Master")
    if (!master_mat ){
        UE_LOG(LogTemp, Error, TEXT("Failed to load the Material"));
        return nullptr ;
    }

    //FString   package_path = TEXT("/Game/Materials/MI_Grass");
    //
    FString     package_path = TsUtil::GetPackagePath( asset_name ) ;
    FString     package_name = FPackageName::ObjectPathToPackageName(package_path);
    UPackage*   package      = CreatePackage(*package_name);

    //
    material = NewObject<UMaterialInstanceConstant>(
                    package,
                    *asset_name,//FName("MI_Grass"),
                    RF_Public | RF_Standalone | RF_Transactional
               );
    material->SetParentEditorOnly(master_mat); //

    //
    //Using the TMap to set the Texture parameters.
    // 
    //UTexture* tex_asset = LoadObject<UTexture>(nullptr, TEXT("/Game/Textures/T_Grass.T_Grass"));
    //static const FName TextureParamName("BaseTexture");
    //material->SetTextureParameterValueEditorOnly(TextureParamName, tex_asset);

    //
    material->PostEditChange();
    material->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(material);

    FString fname = FPackageName::LongPackageNameToFilename(package_name, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs save_args;
    save_args.TopLevelFlags = RF_Public | RF_Standalone;
    save_args.SaveFlags = SAVE_NoError;
    UPackage::SavePackage(package, material, *fname, save_args);

    UE_LOG(LogTemp, Log, TEXT("Material Instance '%s' Saved suceessfully"), *package_path);
#endif
    return material;
}
