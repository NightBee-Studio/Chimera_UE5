#include "TsMaterial.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

#include "TsUtility.h"


// Your original Build function rewritten to match UE5.4-style StaticMesh LOD/mesh description usage
void TsMaterial::Build(
    const FString& msmat_path,
    const FString& asset_name)
{
#if WITH_EDITOR
    FString package_path = TsUtil::GetPackagePath( asset_name ) ;

#endif
}
