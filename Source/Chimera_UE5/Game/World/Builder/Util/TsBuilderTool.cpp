#include "TsBuilderTool.h"

//package関連
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

//Static Mesh
#include "Engine/StaticMesh.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "PhysicsEngine/BodySetup.h"

#include "Engine/Texture2DArray.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceConstant.h"


#include "../Biome/TsBiomeMap.h"



UTexture2D*		 TsBuilderTool::Combine_Texture(
			TMap<int, UTexture2D*>&    textures ,
    		const FString&			   asset_name
	)
{
    int w = 0;
    int h = 0;
    TArray<FColor> OutputPixels;
    for ( auto & tx : textures ){
        w = tx.Value->GetSizeX() ;
        h = tx.Value->GetSizeY() ;
		// mip0 の取得
		const FTexture2DMipMap& mip  = tx.Value->GetPlatformData()->Mips[0];
        EPixelFormat            fmt  = tx.Value->GetPixelFormat();
		// ロックして読み取り
		const uint8* src_data = static_cast<const uint8*>(mip.BulkData.LockReadOnly());
		for (int32 i = 0; i < w*h ; ++i){
			uint8 v = src_data[i * 4]; // 1chテクスチャだとして Red チャンネルを取得
			switch ( tx.Key ){
				case 0: OutputPixels[i].R = v; break;
				case 1: OutputPixels[i].G = v; break;
				case 2: OutputPixels[i].B = v; break;
				case 3: OutputPixels[i].A = v; break;
			}
		}
		mip.BulkData.Unlock();
    }

    FString         package_path = TsUtil::GetPackagePath( asset_name ) ;
    FString         package_name = FPackageName::ObjectPathToPackageName(package_path);
    UPackage*       package      = CreatePackage(*package_name);

    UTexture2D* tex = NewObject<UTexture2D>(package, *asset_name, RF_Public | RF_Standalone);
	tex->AddToRoot();

	FTexture2DMipMap* new_mip = new FTexture2DMipMap();
	new_mip->SizeX = w;
	new_mip->SizeY = h;
	new_mip->BulkData.Lock(LOCK_READ_WRITE);
	void*   dest_data = new_mip->BulkData.Realloc(w * h * sizeof(FColor));
	FMemory::Memcpy(dest_data, OutputPixels.GetData(), w * h * sizeof(FColor));
	new_mip->BulkData.Unlock();

    FTexturePlatformData * pl_data = new FTexturePlatformData();
	pl_data->SizeX = w;
	pl_data->SizeY = h;
	pl_data->PixelFormat = PF_B8G8R8A8;
	pl_data->Mips.Add(new_mip);

    tex->SetPlatformData(pl_data);
	//tex->SRGB = false;
    tex->UpdateResource();

    // アセット登録と保存
    FAssetRegistryModule::AssetCreated(tex);
    package->MarkPackageDirty();

    const FString pkg_filename = FPackageName::LongPackageNameToFilename(package_name, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs save_args;
    save_args.TopLevelFlags = RF_Public | RF_Standalone;
    save_args.SaveFlags = SAVE_NoError;
    save_args.bWarnOfLongFilename = true;
    UPackage::SavePackage(package, tex, *pkg_filename, save_args);

    return tex;
}


UTexture2DArray* TsBuilderTool::Build_TexArray(
		TArray<UTexture2D*>&    textures ,
		const FString&			asset_name
	)
{
    if (textures.Num() == 0) return nullptr;

    FString         package_path = TsUtil::GetPackagePath( asset_name ) ;
    FString         package_name = FPackageName::ObjectPathToPackageName(package_path);
    UPackage*       package      = CreatePackage(*package_name);

    FTexturePlatformData * pl_data = new FTexturePlatformData();
    pl_data->SizeX      = textures[0]->GetSizeX();
    pl_data->SizeY      = textures[0]->GetSizeY();
    pl_data->PixelFormat= textures[0]->GetPixelFormat();
    pl_data->SetNumSlices( textures.Num() ) ;

#if 0

#if WITH_EDITORONLY_DATA
    // 各テクスチャのスライスデータをコピー
    for ( auto t : textures ){
        if (!t || !t->GetPlatformData() || t->GetPlatformData()->Mips.Num() == 0)
            continue;

        // 元テクスチャの Mip0 を取得
        FTexture2DMipMap&   src_mip  = t->GetPlatformData()->Mips[0];
        const int32         mip_size = src_mip.BulkData.GetBulkDataSize();

        // 新しいスライス用の MipMap を作成
        FTexture2DMipMap* new_mip = new FTexture2DMipMap();
        new_mip->SizeX = src_mip.SizeX;
        new_mip->SizeY = src_mip.SizeY;
        new_mip->BulkData.Lock(LOCK_READ_WRITE);

	    if ( !t || !t->Source.IsValid() ) continue ; 
        // バッファ確保してコピー
		const void* src_data = (void*)t->Source.LockMip(0);
        void*       dst_data = new_mip->BulkData.Realloc(mip_size);
        FMemory::Memcpy(dst_data, src_data, mip_size);
        new_mip->BulkData.Unlock();
		t->Source.UnlockMip(0) ;

        // Mipsに追加（注意：これは本来 mip per slice ではなく slice per mip）
        pl_data->Mips.Add(new_mip);
    }
#endif
#else
#if WITH_EDITORONLY_DATA
    const int32 tex_width  = textures[0]->GetSizeX();
    const int32 tex_height = textures[0]->GetSizeY();
    const ETextureSourceFormat src_format = textures[0]->Source.GetFormat();
    const int32 bytes_per_pixel = textures[0]->Source.GetBytesPerPixel();
    const int32 slice_size = tex_width * tex_height * bytes_per_pixel;
    const int32 num_slices = textures.Num();
    const int32 total_size = slice_size * num_slices;

    // すべてのスライスを一括で保持する Mip を作成
    FTexture2DMipMap* new_mip = new FTexture2DMipMap();
    new_mip->SizeX = tex_width;
    new_mip->SizeY = tex_height;
    new_mip->BulkData.Lock(LOCK_READ_WRITE);
    void* dst_data = new_mip->BulkData.Realloc(total_size);
    uint8* dst_ptr = reinterpret_cast<uint8*>(dst_data);

    for (int32 slice_idx = 0; slice_idx < num_slices; ++slice_idx){
        UTexture2D* t = textures[slice_idx];
        if (!t || !t->Source.IsValid())
            continue;

        const void* src_data = t->Source.LockMip(0);
        FMemory::Memcpy(dst_ptr + slice_idx * slice_size, src_data, slice_size);
        t->Source.UnlockMip(0);
    }

    new_mip->BulkData.Unlock();
    pl_data->Mips.Add(new_mip);
#endif

#endif
    // テクスチャ配列生成
    UTexture2DArray* tex_array = NewObject<UTexture2DArray>(package, *asset_name, RF_Public | RF_Standalone);
    tex_array->SetPlatformData(pl_data);
    tex_array->UpdateResource();

    // アセット登録と保存
    FAssetRegistryModule::AssetCreated(tex_array);
    package->MarkPackageDirty();

    const FString pkg_filename = FPackageName::LongPackageNameToFilename(package_name, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs save_args;
    save_args.TopLevelFlags = RF_Public | RF_Standalone;
    save_args.SaveFlags = SAVE_NoError;
    save_args.bWarnOfLongFilename = true;
    UPackage::SavePackage(package, tex_array, *pkg_filename, save_args);

    return tex_array;
}


	 
void	TsBuilderTool::Build_MaterialSet(
		TArray<FTsGroundTex>&	texsets
	)
{
    TArray<UTexture2D*> alb_array ;
    TArray<UTexture2D*> nrm_array ;
    for ( auto& set : texsets ){
        alb_array.Add(set.mTexAlbedo) ;
        nrm_array.Add(set.mTexNormal) ;
    }
    Build_TexArray( alb_array, TEXT("TA_GroundAlbedo") ) ;
    Build_TexArray( nrm_array, TEXT("TA_GroundNormal") ) ;
}



UMaterialInstanceConstant* 
    TsBuilderTool::Build_MaterialInstance(
        const FString& msmat_path,
        const FString& asset_name,
        const TArray<TsGroundSlot>& slots
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
    int idx = 0 ;
    for ( auto& s : slots ){
        FString idx_name = FString::Printf(TEXT("Slot%02d_Index"), idx );
        FString tex_name = FString::Printf(TEXT("Slot%02d_Mask" ), idx );
        material->SetTextureParameterValueEditorOnly( FName(*tex_name), s.mTex ) ;
        material->SetScalarParameterValueEditorOnly ( FName(*idx_name), s.mMat-1 ) ;

        UE_LOG(LogTemp, Log, TEXT("   Material Parameter'%s': idx%02d [%s]"), *idx_name, s.mMat-1 , *StaticEnum<EMaterialType>()->GetNameStringByValue(static_cast<int64>(s.mMat)));

        idx++;
    }
    
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


// Your original Build function rewritten to match UE5.4-style StaticMesh LOD/mesh description usage
void TsBuilderTool::Build_HeightMesh(
    TsBiomeMap*       tex_map,
    const TsUtil::TsBox&tex_rect,
    TsNoiseMap *		noise_map,
    float       		noise_scale,
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

    UE_LOG(LogTemp, Log, TEXT("TsHeightMesh::Build  row%d"),nv );

    tex_map->Lock();
    for (int y = 0; y < nv; ++y){
        for (int x = 0; x < nv; ++x){
            auto    make_pos = [&] (float dx, float dy) {
                FVector2D p = FVector2D( tex_rect.mX + (x+dx) * sx,
                                         tex_rect.mY + (y+dy) * sy) ;
                float h = tex_map->GetPixelTex2D_Catmull( p.X, p.Y ) + noise_map->GetValue( p ) * noise_scale ;
                return FVector3f( (x+dx)*s, (y+dy)*s, h * mesh_height ) ;
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
        if ( (y % 100)==0 ) UE_LOG(LogTemp, Log, TEXT("    %d/%d"),y, nv );
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

    //add collisom
    UBodySetup* body_setup = static_mesh->GetBodySetup();
    if (!body_setup){
        static_mesh->CreateBodySetup();
        body_setup = static_mesh->GetBodySetup();
    }
    body_setup->RemoveSimpleCollision();
    FKAggregateGeom& agg_geom = body_setup->AggGeom;
    agg_geom.BoxElems.Add(FKBoxElem(50.0f, 50.0f, 50.0f));  // サイズは任意調整
    body_setup->CollisionTraceFlag = CTF_UseComplexAsSimple;
    body_setup->bDoubleSidedGeometry = true;
    body_setup->InvalidatePhysicsData();
    body_setup->CreatePhysicsMeshes();

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

