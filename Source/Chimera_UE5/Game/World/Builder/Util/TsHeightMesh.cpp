#include "Engine/StaticMesh.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

#include "TsHeightMesh.h"

// Your original Build function rewritten to match UE5.4-style StaticMesh LOD/mesh description usage
void TsHeightMesh::Build(
    TsTextureMap*       tex_map,
    const TsUtil::TsBox&tex_rect,
    int                 mesh_div,
    float               mesh_size,
    float               mesh_height,
    const FString&      asset_name,
	UMaterialInterface*	material )
{
#if WITH_EDITOR
    const int n = mesh_div;
    const int nv = n + 1;
    const float s  = mesh_size   / n ;
    const float sx = (float)tex_rect.mW / n ;
    const float sy = (float)tex_rect.mH / n ;

    TArray<FVector3f> vt_list;
    TArray<FVector3f> nm_list;
    TArray<FVector2f> uv_list;
    TArray<int32    > pl_list;

    tex_map->Lock();
    for (int y = 0; y < nv; ++y){
        for (int x = 0; x < nv; ++x){
            auto    make_pos = [&] (float dx, float dy) {
                float px = tex_rect.mX + (x+dx) * sx;
                float py = tex_rect.mY + (y+dy) * sy;
                return FVector3f( (x+dx)*s, (y+dy)*s, tex_map->GetValue(px, py) * mesh_height ) ;
            } ;

            vt_list.Add( make_pos(0,0) );
            uv_list.Add(FVector2f((float)x / n, (float)y / n));

            float g = 0.2f ;
            FVector3f p00 = make_pos(-g,-g) ;
            FVector3f p01 = make_pos(-g,+g) ;
            FVector3f p10 = make_pos(+g,-g) ;
            FVector3f p11 = make_pos(+g,+g) ;
            FVector3f n00 = FVector3f::CrossProduct(p01-p00, p10-p00).GetSafeNormal();
            FVector3f n11 = FVector3f::CrossProduct(p01-p11, p10-p11).GetSafeNormal();
            n00 *= n00.Z<0 ? -1 : 1 ;
            n11 *= n11.Z<0 ? -1 : 1 ;
            nm_list.Add( (n00+n11).GetSafeNormal() ) ;
        }
    }
    tex_map->UnLock();

    for (int y = 0; y < n; ++y){
        for (int x = 0; x < n; ++x){
            int i00 = y * nv + x;
            int i10 = i00 + 1;
            int i01 = i00 + nv;
            int i11 = i01 + 1;

            pl_list.Add(i00); pl_list.Add(i01); pl_list.Add(i10);
            pl_list.Add(i10); pl_list.Add(i01); pl_list.Add(i11);
        }
    }

    FString         package_path = TsUtil::GetPackagePath( asset_name ) ;
    FString         package_name = FPackageName::ObjectPathToPackageName(package_path);
    UPackage*       package      = CreatePackage(*package_name);
    UStaticMesh*    static_mesh  = NewObject<UStaticMesh>(package, *asset_name, RF_Public | RF_Standalone);
    static_mesh->SetNumSourceModels(1);

    const int32 LODIndex = 0;
    FStaticMeshSourceModel& src_model = static_mesh->GetSourceModel(LODIndex);
    src_model.BuildSettings.bRecomputeNormals   = false;
    src_model.BuildSettings.bRecomputeTangents  = false;
    src_model.BuildSettings.bUseMikkTSpace      = true;

    FMeshDescription*       mesh_desc = static_mesh->CreateMeshDescription(LODIndex);
    FStaticMeshAttributes attributes(*mesh_desc);
    attributes.Register();

    TMap<int32, FVertexID        >          vert_vidmap;
    TMap<int32, FVertexInstanceID>          vert_iidmap;

    TVertexAttributesRef<FVector3f>         VertexPositions = attributes.GetVertexPositions();
    TVertexInstanceAttributesRef<FVector2f> VertexUVs       = attributes.GetVertexInstanceUVs();
    TVertexInstanceAttributesRef<FVector3f> VertexNormals   = attributes.GetVertexInstanceNormals();
    TVertexInstanceAttributesRef<FVector3f> VertexTangents  = attributes.GetVertexInstanceTangents();
    TVertexInstanceAttributesRef<float    > VertexBinormals = attributes.GetVertexInstanceBinormalSigns();
    TVertexInstanceAttributesRef<FVector4f> VertexColors    = attributes.GetVertexInstanceColors();

    VertexUVs.SetNumChannels(1);

    for (int i = 0; i < vt_list.Num(); i++){
        FVertexID           v_id = mesh_desc->CreateVertex();
        FVertexInstanceID   i_id = mesh_desc->CreateVertexInstance(v_id);
        VertexPositions [v_id] = vt_list[i];
        VertexUVs       [i_id] = uv_list[i];
        VertexNormals   [i_id] = nm_list[i];
        VertexTangents  [i_id] = FVector3f::VectorPlaneProject(FVector3f(1,0,0), nm_list[i]).GetSafeNormal();
        VertexBinormals [i_id] = 1.0f;
        VertexColors    [i_id] = FVector4f(1, 1, 1, 1);

        vert_vidmap.Add(i, v_id);
        vert_iidmap.Add(i, i_id);
    }

    FPolygonGroupID     ply_group       = mesh_desc->CreatePolygonGroup();
    for (int i = 0; i < pl_list.Num(); i += 3){
        TArray<FVertexInstanceID> tri = {
            vert_iidmap[pl_list[i+0]],
            vert_iidmap[pl_list[i+1]],
            vert_iidmap[pl_list[i+2]]
        };
        mesh_desc->CreatePolygon( ply_group, tri );
    }

    if ( !material ) material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
    static_mesh->GetStaticMaterials().Add( FStaticMaterial(material) );

    static_mesh->NaniteSettings.bEnabled = true;

    static_mesh->CommitMeshDescription(LODIndex);
    static_mesh->ImportVersion = EImportStaticMeshVersion::LastVersion;
    static_mesh->Build();
    static_mesh->MarkPackageDirty();
    static_mesh->PostEditChange();

    FAssetRegistryModule::AssetCreated(static_mesh);
    package->SetDirtyFlag(true);

    FString pkg_filename = FPackageName::LongPackageNameToFilename(package_name, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs save_args;
    save_args.TopLevelFlags = RF_Public | RF_Standalone;
    save_args.SaveFlags = SAVE_NoError;
    save_args.bWarnOfLongFilename = true;
    UPackage::SavePackage(package, static_mesh, *pkg_filename, save_args);
#endif
}
