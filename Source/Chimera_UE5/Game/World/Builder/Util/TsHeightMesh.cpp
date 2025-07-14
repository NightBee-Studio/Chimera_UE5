#include "TsHeightMesh.h"

#include "ProceduralMeshComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Engine/StaticMesh.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"




void TsHeightMesh::Build(UObject* outer, TsTextureMap* heightmap,int div, const FString& packagename, const FString& assetname )
{
    // 頂点生成
    TArray<FVector3f> vt_list;
    TArray<FVector2f> uv_list;
    TArray<int      > pl_list;

    heightmap->Lock() ;
#define ScaleXY  10.0f
    for ( int y=0; y<div; ++y ){
        for ( int x=0; x<div; ++x ){
            vt_list.Add( FVector3f( x * ScaleXY, y * ScaleXY, heightmap->GetPixel(x,y) ));
            uv_list.Add( FVector2f( (float)x/div, (float)y/div ));
        }
    }
    heightmap->UnLock() ;

    // 三角形のインデックス生成
   for ( int y=0; y<div; ++y ){
        for ( int x=0; x<div; ++x ){
            int i00 = y*div + x;
            int i10 = i00 + 1;
            int i01 = i00 + div;
            int i11 = i01 + 1;
            pl_list.Add(i00); pl_list.Add(i01); pl_list.Add(i10);
            pl_list.Add(i10); pl_list.Add(i01); pl_list.Add(i11);
        }
    }

    FString      full_packagename   = FPackageName::ObjectPathToPackageName(packagename);
    UPackage*    package            = CreatePackage(*full_packagename);
    UStaticMesh* static_mesh = NewObject<UStaticMesh>(package, *assetname, RF_Public | RF_Standalone);
    static_mesh->InitResources();

    // MeshDescriptionの生成
    FMeshDescription *      mesh_desc = static_mesh->GetMeshDescription(0);
    FStaticMeshAttributes   attr(*mesh_desc);
    attr.Register();

    TMap<int, FVertexID        >  vert_vidmap;
    TMap<int, FVertexInstanceID>  vert_iidmap;
	TVertexAttributesRef<FVector3f>         verts = attr.GetVertexPositions();
    TVertexInstanceAttributesRef<FVector3f> norms = attr.GetVertexInstanceNormals();
	TVertexInstanceAttributesRef<FVector2f> uvs   = attr.GetVertexInstanceUVs();

    for (int i=0; i<vt_list.Num(); i++){
        FVertexID           v_id = mesh_desc->CreateVertex();
        FVertexInstanceID   i_id = mesh_desc->CreateVertexInstance(v_id);

        verts[v_id] = vt_list[i];
        uvs  [v_id] = uv_list[i];
        vert_vidmap.Add(i,v_id);
        vert_iidmap.Add(i,i_id);
    }

    for (int i=0; i<pl_list.Num() ; i += 3){
        mesh_desc->CreatePolygon(
            mesh_desc->CreatePolygonGroup(), 
            {   vert_iidmap[pl_list[i+0]],
                vert_iidmap[pl_list[i+1]],
                vert_iidmap[pl_list[i+2]]  });
    }

    // StaticMesh アセットを作成
    static_mesh->CommitMeshDescription(0);              // コミット
    static_mesh->Build();

    FAssetRegistryModule::AssetCreated( static_mesh );
    package->SetDirtyFlag(true);

#if WITH_EDITOR
    FSavePackageArgs save_args;

    save_args.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
    save_args.SaveFlags     = SAVE_NoError;

    if ( GEditor && GEditor->SavePackage(package, static_mesh, *full_packagename, save_args) ){
        //UE::SavePackage(
        //        package,
        //        static_mesh,
        //        EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
        //        *FPackageName::LongPackageNameToFilename( full_packagename, FPackageName::GetAssetPackageExtension() )
        //    );
    }
#endif
}
