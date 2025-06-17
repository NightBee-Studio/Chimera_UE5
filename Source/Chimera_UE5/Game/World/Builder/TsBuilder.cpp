
#include "TsBuilder.h"

#include "Biome/TsBiome.h"
#include "Biome/TsBiomeMap.h"
#include "Biome/TsBiomeSite.h"
#include "Biome/TsBiomeSurface.h"

#include "Util/TsErosion.h"
#include "Util/TsVoronoi.h"
#include "Util/TsUtility.h"




// ---------------------------- Builder -------------------------

class Builder_Work {
public:
	TMap<EBiomeSType, TsBiomeSurface>	mSurfaces;
	TArray<TsBiome>						mBiomes;
	TsBiomeSite*						mShape;

	FBox2D								mBoundingbox;

	TsMapOutput							mMapOutParam;
	TsHeightMap*						mHeightMap;
	TsNormalMap*						mNormalMap;
	TsMaterialMap*						mMaterialMap;

private:
	TsBiome*		SearchBiome(const FVector2D& p)		// world-coord
	{
		for (auto& b : mBiomes) {
			if (b.IsInside(p)) return &b;
		}
		return nullptr;
	}

	void			SetOutputConfig(const TsMapOutput& conf) { mMapOutParam = conf; }

	const FBox2D&	GetBoundingbox() const	{ return mBoundingbox ; }
	TsHeightMap*	GetHeightMap() const	{ return mHeightMap   ; }
	TsMaterialMap*	GetMaterialMap() const	{ return mMaterialMap ; }

#define SAFE_DELETE( var )	{if (var){ delete (var);(var)=nullptr;}}
	void			Release()
	{
		mBiomes.Empty();
		mShape->Release();

		SAFE_DELETE(mHeightMap);
		SAFE_DELETE(mNormalMap);
		SAFE_DELETE(mMaterialMap);
	}

public:
	void			Debug(UWorld* world)
	{
		mShape->Debug(world);

		for (auto &b : mBiomes) {
			FColor c(0, 0, 0);
			switch (b.GetSType()) {
			case EBiomeSType::E_SurfNone:		c = FColor(  0,   0, 180); break;
			case EBiomeSType::E_SurfLake:		c = FColor(100, 120, 255); break;
			case EBiomeSType::E_SurfField:		c = FColor(125, 255,   0); break;
			case EBiomeSType::E_SurfMountain:	c = FColor(120,  60,  20); break;
			}
			b.Debug(world, c);
		}
	}

	Builder_Work()
		: mShape( new TsBiomeSite() )
		, mHeightMap(nullptr)
		, mNormalMap(nullptr)
		, mMaterialMap(nullptr)
	{;}

	void			BuildLandscape(
		float _x, float _y, float radius,
		int		seed,
		float	voronoi_size,
		float	voronoi_jitter,
		int		heightmap_reso,
		int		erode_cycle)
	{
		TsUtil::RandSeed( seed );

		//clean up first.
		Release();

		// create island shape
		mShape->Generate(_x, _y, radius);

		{//-------------------------------------------------------------------------------------- create voronois
			mShape->UpdateBoundingbox(mBoundingbox);
			mBoundingbox.Min -= FVector2D(500, 500);
			mBoundingbox.Max += FVector2D(500, 500);

			TsVoronoiSite<TsBiome> voronoi_site;
			voronoi_site.GenerateSite(
					mBiomes, 
					mBoundingbox,
					voronoi_size,
					voronoi_jitter
				);
			voronoi_site.Tesselate( mBiomes );

			for (auto& b : mBiomes) {
				b.SetOwner();
			}
		}

		{///-------------------------------------------------------------------------------- Biome Group 		// you can access by 2d
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Biome Group"));
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: BiomeMap creating ..."));

			heightmap_reso = 1024;
			//heightmap_reso = 2048;
			if (heightmap_reso == 0) heightmap_reso = 512;
#define IMG_SIZE heightmap_reso

			mMapOutParam = TsMapOutput(0, 0, heightmap_reso, 1);
			TsImageCore::SetDirectory("Resources\\World\\Landscape\\Surface\\", mMapOutParam.x, mMapOutParam.y);

			mSurfaces = TMap<EBiomeSType, TsBiomeSurface>{
				{ EBiomeSType::E_SurfNone,
					TsBiomeSurface({
							new TsSurfaceField	  (1.0f, 5.0f     ),
							new TsSurfacePondNoise( TsNoiseParam({{0.8f,0.01f},{0.2f,0.03f}}), 0.5f, 0.4f),
						},
						new TsBiomeMFunc(
							EBiomeMapType::E_Moist,{
								{ 1.00f,0.0f, EMaterialType::MT_BaseLand  },
							})
					)
				},
				{ EBiomeSType::E_SurfLake,
					TsBiomeSurface({
						new TsSurfaceLake(TsNoiseParam({{1.0f, 0.001f},{ 0.2f, 0.003f}}), -10.0f),
					},
						new TsBiomeMFunc(
							EBiomeMapType::E_Moist,{
								{ 0.60f, 0.0f, EMaterialType::MT_LakeSoil_A },
								{ 0.50f, 0.0f, EMaterialType::MT_Soil_B     },
							})
					)
				},
				{ EBiomeSType::E_SurfField,
					TsBiomeSurface({
							new TsSurfaceField	  (1.00f, 10.0f     ),
							new TsSurfacePondNoise(TsNoiseParam({{0.40f,0.01f},{0.15f,0.03f}}), 0.5f, 0.4f),
						},
						new TsBiomeMFunc(
							EBiomeMapType::E_Moist,{
								{ 0.05f, 0.0f, EMaterialType::MT_Soil_A   },
								{ 0.10f, 0.0f, EMaterialType::MT_Soil_B   },
								{ 0.10f, 0.0f, EMaterialType::MT_Grass_A  },
								{ 0.35f, 0.0f, EMaterialType::MT_Grass_B  },
								{ 0.10f, 0.0f, EMaterialType::MT_Forest_A },
								{ 0.15f, 0.0f, EMaterialType::MT_Forest_B },
							})
					)
				},
				{ EBiomeSType::E_SurfMountain,
					TsBiomeSurface({
							new TsSurfaceMountain ( 20       ),
							//new TsSurfacePondNoise(TsNoiseParam({{0.8f , 0.010f},{ 0.2f, 0.030f}}), 0.5f, 0.5f),
							//new TsSurfaceNoise    (10.0f, 2.0f, 1.4f),
						},
						new TsBiomeMFunc(
							EBiomeMapType::E_Moist,{
								{  0.20f, 0.0f, EMaterialType::MT_Grass_A },
								{  0.35f, 0.0f, EMaterialType::MT_Grass_B },
								{  0.10f, 0.0f, EMaterialType::MT_Soil_B  },
								{  0.45f, 0.0f, EMaterialType::MT_Soil_A  },
							})
					)
				},
			} ;

			////// apply the biome from PerlinNoise
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Apply Surface ..."));

			TsBiomeMap* surfc_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam({ {1.0f, 0.0010f},{0.2f, 0.0030f} }));
			TsBiomeMap* genre_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam({ {1.0f, 0.0010f},{0.2f, 0.0030f} }));
#define S	1.0f 
			TsBiomeMap* moist_map =
				new TsBiomeMap(
					IMG_SIZE, IMG_SIZE,
					&mBoundingbox, 
					TsNoiseParam({
						{ 0.80f, 0.001f*S },
						{ 0.50f, 0.002f*S },
						{ 0.25f, 0.004f*S },
						{ 0.13f, 0.008f*S },
						{ 0.60f, 0.016f*S },
						{ 0.30f, 0.032f*S },
					})
				);
#undef S


			{///---------------------------------------- Setup Biome
				// world once
				TArray<TsBiomeItem_SType> surfc_items = {
					{ 0.05f, 0.0f, EBiomeSType::E_SurfLake    },
					{ 0.70f, 0.0f, EBiomeSType::E_SurfField   },
					{ 0.25f, 0.0f, EBiomeSType::E_SurfMountain},
				};
				TArray<TsBiomeItem_MType> moist_items = {
					{ 0.10f, 0.0f, EBiomeMType::E_Soil	  },
					{ 0.35f, 0.0f, EBiomeMType::E_Field	  },
					{ 0.05f, 0.0f, EBiomeMType::E_Tree	  },
					{ 0.30f, 0.0f, EBiomeMType::E_ForestA },
					{ 0.15f, 0.0f, EBiomeMType::E_ForestB },
				};

				surfc_map->SetupItems< TsBiome, TsBiomeItem_SType >(mBiomes, surfc_items);
				moist_map->SetupItems< TsBiome, TsBiomeItem_MType >(mBiomes, moist_items);
				
				for ( auto& b : mBiomes ) {
					if (mShape->IsInside(b)) {
						b.SetSType( surfc_map->SelectItem<TsBiome, TsBiomeItem_SType>( b, surfc_items).mItem );
						b.SetMType( moist_map->SelectItem<TsBiome, TsBiomeItem_MType>( b, moist_items).mItem );
					}
				}
			}

			{///---------------------------------------- Generate Mapping
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Generate Mappings ..."));
				int		reso  = mMapOutParam.LocalReso();
				FBox2D	bound = mMapOutParam.LocalBound(mBoundingbox);

				mHeightMap           = new TsHeightMap  ( reso, reso, &bound );
				mNormalMap	         = new TsNormalMap  ( reso, reso, &bound );// only for output
				mMaterialMap         = new TsMaterialMap( reso, reso, &bound,{
						EMaterialType::MT_BaseLand,
						EMaterialType::MT_LakeSoil_A,
						EMaterialType::MT_Sand_A,
						EMaterialType::MT_Sand_B,
						EMaterialType::MT_Soil_A,
						EMaterialType::MT_Soil_B,
						EMaterialType::MT_Snow_A,
						EMaterialType::MT_Grass_A,
						EMaterialType::MT_Grass_B,
						EMaterialType::MT_Forest_A,
						EMaterialType::MT_Forest_B,
						EMaterialType::MT_Rock_A,
						EMaterialType::MT_Moss_A,
						EMaterialType::MT_Moss_B,
					});
				TsBiomeMap* flow_map = new TsBiomeMap   ( reso, reso, &bound );
				TsBiomeMap* pond_map = new TsBiomeMap   ( reso, reso, &bound );

				TsBiomeMap::AddBiomeMap( EBiomeMapType::E_Tempr , surfc_map );
				TsBiomeMap::AddBiomeMap( EBiomeMapType::E_Moist , moist_map );
				TsBiomeMap::AddBiomeMap( EBiomeMapType::E_Flow  , flow_map  );	//flow
				TsBiomeMap::AddBiomeMap( EBiomeMapType::E_Pond  , pond_map  );	//pond

#if 1
				{///---------------------------------------- HeightMap
					{///--------------------------------------------------- Create BaseHeightmap
						UE_LOG(LogTemp, Log, TEXT("   Base Heightmap start ...."));
						mHeightMap->ForeachPixel(
							[&](int px, int py) {
								FVector2D p = mHeightMap->GetWorldPos(px, py);
								if (mHeightMap->IsWorld(p)) {
									if (TsBiome* b = SearchBiome(p)) {
										mSurfaces[b->GetSType()].UpdateRemap(b, p);
										if (b->GetSType() == EBiomeSType::E_SurfMountain || b->GetSType() == EBiomeSType::E_SurfLake) {
											mSurfaces[EBiomeSType::E_SurfField].UpdateRemap(b, p);
										}
									}
								}
							} );
						UE_LOG(LogTemp, Log, TEXT("   Base Heightmap remap done."));
						mHeightMap->ForeachPixel(
							[&](int px, int py) {
								FVector2D p = mHeightMap->GetWorldPos(px, py);
								if (TsBiome* b = SearchBiome(p)) {
									float h = mSurfaces[b->GetSType()].GetHeight(b, p);
									if (b->GetSType() == EBiomeSType::E_SurfMountain || b->GetSType() == EBiomeSType::E_SurfLake) {
										h += mSurfaces[EBiomeSType::E_SurfField].GetHeight(b, p);
									}
									mHeightMap->SetPixel(px, py, h);
								}
							});
						mHeightMap->Normalize();		// normalize to 0-1

						UE_LOG(LogTemp, Log, TEXT("   Base Heightmap done."));
					}

					{///--------------------------------------------------- Erosion
						UE_LOG(LogTemp, Log, TEXT("   Erosion start..."));

						erode_cycle = 0 ;
						if (erode_cycle > 0) {
							TsErosion(mHeightMap, flow_map, pond_map).Simulate(erode_cycle);
						} else{
							UE_LOG(LogTemp, Log, TEXT("       skipped..."));
						}

						UE_LOG(LogTemp, Log, TEXT("   Erosion done."));
					}

					{///--------------------------------------------------- Normal
						UE_LOG(LogTemp, Log, TEXT("   Normal start..."));

						mHeightMap->ForeachPixel(
							[&](int px, int py) {
								mNormalMap->SetPixel(px, py, mHeightMap->CalcNormal(px, py, 4));
							});

						UE_LOG(LogTemp, Log, TEXT("   Normal done."));
					}

					mHeightMap->Save("HeightMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
					mHeightMap->Save("HeightMap.raw", EImageFile::Raw, EImageFormat::FormatL16);
					mNormalMap->Save("NormalMap.dds", EImageFile::Dds, EImageFormat::FormatB8G8R8A8);

					UE_LOG(LogTemp, Log, TEXT("   HeightMap done."));
				}
#endif 
				{
					UE_LOG(LogTemp, Log, TEXT("BiomeSystem:: MaterialMap preparing..."));
					
#define S	3.0f 
					TsBiomeMap* trans_map = 
						new TsBiomeMap(
							IMG_SIZE, IMG_SIZE,
							&mBoundingbox, 
							TsNoiseParam({
								{ 0.80f, 0.001f * S },
								{ 0.50f, 0.002f * S },
								{ 0.25f, 0.004f * S },
								{ 0.13f, 0.008f * S },
								{ 0.60f, 0.016f * S },
								{ 0.30f, 0.032f * S },
							}) 
						);
#undef S
					trans_map->ForeachPixel(
						[&](int px, int py) {
							trans_map->UpdateRemap( trans_map->GetWorldPos(px, py) );
						});

					TArray<TsBiomeItem_Material> moist_items = {
						{ 0.08f, 0.0f, EMaterialType::MT_Soil_A,},
						{ 0.02f, 0.0f, EMaterialType::MT_Soil_B,},
						{ 0.35f, 0.0f, EMaterialType::MT_Grass_A, },
						{ 0.15f, 0.0f, EMaterialType::MT_Grass_B, },
						{ 0.30f, 0.0f, EMaterialType::MT_Forest_A, },
						{ 0.15f, 0.0f, EMaterialType::MT_Forest_B, },
					};
					bool first = true;
					moist_map->SetupItemsPixel< TsBiomeItem_Material >(moist_items);
					moist_map->ForeachPixel(
						[&](int px, int py) {
							TsMaterialPixel	pix;

							FVector2D p = moist_map->GetWorldPos(px, py);
							if (moist_map->IsWorld(p)) {
								TsUtil::ForeachGaussian(p, moist_map->GetStep(),
									[&](const FVector2D& pos, float weight) {
										int idx = moist_map->SelectItemIdx<FVector2D, TsBiomeItem_Material>(pos, moist_items);
										pix.Add( moist_items[idx].mItem, weight);
									}); 
							}
							mMaterialMap->MergePixel(px, py, pix);
						});
					UE_LOG(LogTemp, Log, TEXT("BiomeSystem:: MaterialMap prepare done."));

					//mMaterialMap->ForeachPix(
					//	[&](int px, int py, TsMaterialPixel& pix) {
					//		for (auto& mv : pix.mValues) {
					//			if ( 0.0f < mv.Value && mv.Value < 1.0f ) {
					//				FVector2D p = mMaterialMap->GetWorldPos(px, py) ;
					//				float tm = trans_map->Remap( trans_map->GetValue( p ) );
					//				UE_LOG(LogTemp, Log, TEXT("  [%d,%d] TransMsk%f  Value%f"), px, py, tm, mv.Value);
					//				mv.Value = tm < mv.Value ? 1.0f : 0.0f;
					//			}
					//		}
					//	});

					UE_LOG(LogTemp, Log, TEXT("BiomeSystem:: MaterialMap exporting..."));
					mMaterialMap->StoreMaterial();
					mMaterialMap->SaveAll(0, 0, 0, 0);
					UE_LOG(LogTemp, Log, TEXT("BiomeSystem:: MaterialMap done."));
				}

#if 0		//material_map
				{///--------------------------------------------------- MaterialMap
					// update Occupancy of the config...
					UE_LOG(LogTemp, Log, TEXT("BiomeSystem:: MaterialMap preparing..."));
					for (auto& sf : mSurfaces) {
						if (sf.Value.mMFuncs){
							moist_map->SetupItems< TsBiome, TsBiomeItem_Material >(mBiomes, sf.Value.mMFuncs->mItems );
						}
					}

					auto surf_to_mat = [&](EBiomeSType typ) {
							switch (typ) {
							case EBiomeSType::E_SurfLake:		return EMaterialType::MT_Sand_A; 
							case EBiomeSType::E_SurfField:		return EMaterialType::MT_Grass_A; 
							case EBiomeSType::E_SurfMountain:	return EMaterialType::MT_Forest_A;
							case EBiomeSType::E_SurfNone:		return EMaterialType::MT_BaseLand;
							}
							return EMaterialType::MT_BaseLand;
						};
					moist_map->ForeachPixel(
						[&](int px, int py) {
							FVector2D     p = moist_map->GetWorldPos(px, py);
							bool done = false;
							if (moist_map->IsWorld(p)) {
								TsMaterialPixel	pix;
#if 0
								TsUtil::ForeachGaussian( p, moist_map->GetStep(),
									[&](const FVector2D& pos, float weight) {
										if (TsBiome* b = SearchBiome(pos)) {
											pix.Add( surf_to_mat(b->GetSType()), weight );
											//UE_LOG(LogTemp, Log, TEXT("(%d,%d)[%f %f]  Surf[%d]  ty%d w%f"), px, py, pos.X, pos.Y, b->GetSType(), it.mItem, weight);
										}
									} );
								pix.Normalize();
								mMaterialMap->MergePixel(px, py, pix);

#if 1
								if (TsBiome* b = SearchBiome(p) ){
									if (mSurfaces[b->GetSType()].mMFuncs) {
										//TsMaterialPixel	pix;

										//TsUtil::ForeachGaussian( p, moist_map->GetStep(),
										//	[&](const FVector2D& pos, float weight) {
										//		TsBiomeItem_Material it =
										//			moist_map->SelectItem<FVector2D, TsBiomeItem_Material>( pos, mSurfaces[b->GetSType()].mMFuncs->mItems );
										//		pix.Add( it.mItem, weight );
										//		UE_LOG(LogTemp, Log, TEXT("(%d,%d)[%f %f]  Surf[%d]  ty%d w%f"), px, py, pos.X,pos.Y, b->GetSType(), it.mItem, weight);
										//	} );
										//pix.Normalize();
										//mMaterialMap->MergePixel(px, py, pix);

									}
									mMaterialMap->SetPixel(px, py, surf_to_mat(b->GetSType()), 1.0f);
								}
#endif
#endif
								mMaterialMap->MergePixel(px, py, pix);

								done = true;
							}
							if ( !done ) mMaterialMap->SetPixel(px, py, EMaterialType::MT_BaseLand, 1.0f);
						});
					UE_LOG(LogTemp, Log, TEXT("BiomeSystem:: MaterialMap prepare done."));

					UE_LOG(LogTemp, Log, TEXT("BiomeSystem:: MaterialMap exporting..."));
					mMaterialMap->StoreMaterial();
					mMaterialMap->SaveAll(0,0,0,0);
					UE_LOG(LogTemp, Log, TEXT("BiomeSystem:: MaterialMap done."));
				}
#endif		//material_map

				{///--------------------------------------------------- File Exporting 
					UE_LOG(LogTemp, Log, TEXT("    File exporting ..."));

					TsImageCore::SetDirectory("Resources\\World\\Landscape\\Surface\\", mMapOutParam.x, mMapOutParam.y);

					moist_map->ForeachPixel([&](int px, int py) {
						moist_map->SetPixel(px, py, moist_map->GetValue( moist_map->GetWorldPos(px, py) ));
						});
					surfc_map->ForeachPixel([&](int px, int py) {
						surfc_map->SetPixel(px, py, surfc_map->GetValue(surfc_map->GetWorldPos(px, py)));
						});

					moist_map->Save("BM_MoistMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
					moist_map->Save("BM_MoistMap.raw", EImageFile::Raw, EImageFormat::FormatL16);
					surfc_map->Save("BM_SurfcMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
					surfc_map->Save("BM_SurfcMap.raw", EImageFile::Raw, EImageFormat::FormatL16);
					flow_map ->Save("BM_FlowMap.dds" , EImageFile::Dds, EImageFormat::FormatL16);
					flow_map ->Save("BM_FlowMap.raw" , EImageFile::Raw, EImageFormat::FormatL16);
					pond_map ->Save("BM_PondMap.dds" , EImageFile::Dds, EImageFormat::FormatL16);
					pond_map ->Save("BM_PondMap.raw" , EImageFile::Raw, EImageFormat::FormatL16);

					UE_LOG(LogTemp, Log, TEXT("    File exporting done."));
				}
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Generate Mappings  done."));
			}
		}
	}
};




// -------------------------------- UTsLandscape  --------------------------------

ATsBuilder::ATsBuilder()
#if		WITH_EDITOR
	: mImplement( new Builder_Work() )
#else
	: mImplement( nullptr )
#endif	//WITH_EDITOR
{
}



//Radius		700.0f
//VoronoiSize	200.0f
//VoronoiJitter	0.5f

void	ATsBuilder::Build()
{
#if		WITH_EDITOR
	Builder_Work * work = (Builder_Work*)mImplement;

	FVector  pos = GetActorLocation();

	work->BuildLandscape(
		pos.X, pos.Y, mRadius,
		123789,
		mVoronoiSize, mVoronoiJitter,
		mReso,
		mErodeCycle
	);

	work->Debug( GetWorld());

#endif	//WITH_EDITOR

}

