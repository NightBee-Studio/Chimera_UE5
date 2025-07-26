	
#include "TsBuilder.h"

#include "Biome/TsBiome.h"
#include "Biome/TsBiomeMap.h"
#include "Biome/TsBiomeModel.h"
#include "Biome/TsBiomeSite.h"
#include "Biome/TsBiomeSurface.h"

#include "Util/TsErosion.h"
#include "Util/TsVoronoi.h"
#include "Util/TsUtility.h"
//#include "Util/TsMaterial.h"
#include "Util/TsTextureMap.h"
#include "Util/TsHeightMesh.h"




// ---------------------------- Builder -------------------------

class Builder_Work {
public:
	TMap<EBiomeSType, TsBiomeSurface>	mSurfaces;
	TArray<TsBiome>						mBiomes;
	TArray<TsBiomeGroup>				mBiomeGroups ;

	TsBiomeSite							mShape;

	FBox2D								mBoundingbox;

	TsMapOutput							mMapOutParam;
	TsHeightMap*						mHeightMap;
	TsMaterialMap*						mMaterialMap;

private:
	TsBiome*		SearchBiome(const FVector2D& p)		// world-coord
	{
		for (auto& b : mBiomes) {
			if (b.IsInside(p)) return &b;
		}
		return nullptr;
	}
	TsBiomeGroup*	SearchBiomeGroup(const FVector2D& p)// world-coord
	{
		for (auto& bg : mBiomeGroups) {
			if (bg.IsInside(p)) return &bg;
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

		SAFE_DELETE(mHeightMap);
		SAFE_DELETE(mMaterialMap);
	}

public:
	void			Debug(UWorld* world)
	{
		mShape.Debug(world);

		for (auto &b : mBiomes) {
			FColor c(0, 0, 0);
			switch (b.GetSType()) {
			case EBiomeSType::EBSf_None:		c = FColor(  0,   0, 180); break;
			case EBiomeSType::EBSf_Lake:		c = FColor(100, 120, 255); break;
			case EBiomeSType::EBSf_Field:		c = FColor(125, 255,   0); break;
			case EBiomeSType::EBSf_Mountain:	c = FColor(255, 255, 255); break;
			}
			b.Debug(world, c);
		}
	}

	Builder_Work()
		: mShape()
		, mHeightMap(nullptr)
		, mMaterialMap(nullptr)
	{;}

	void			BuildLandscape(
			float _x, float _y, float radius,
			int			seed,
			float		voronoi_size,
			float		voronoi_jitter,
			int			heightmap_reso,
			int			erode_cycle,

			int			unit_size,
			int			unit_div ,
			int			unit_reso,

			TMap<TEnumAsByte<ETextureMap>,TObjectPtr<UTexture2D>>&	texture_maps,

			UDataTable*	biome_specs
		)
	{
		TsUtil::RandSeed( seed );

		UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Build Start.... center(%f,%f) radius%f"), _x, _y, radius );

		//clean up first.
		Release();

		// create island shape
		mShape.Generate(_x, _y, radius,
			{TsBiomeSite::CircleConf(1.0f,1.5f, 0.06f,0.4f, 3, -40, 40),
			 TsBiomeSite::CircleConf(0.8f,1.0f, 0.06f,0.5f, 1,  20, 40),
			 TsBiomeSite::CircleConf(0.7f,0.9f, 0.06f,0.5f, 1, -40,-20),
			 TsBiomeSite::CircleConf(1.0f,1.2f, 0.06f,0.5f, 0, -40,-20),} );

		{//-------------------------------------------------------------------------------------- create voronois
			mShape.UpdateBoundingbox(mBoundingbox);
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

		//	heightmap_reso = 2048;
			heightmap_reso = 1024 ;
			if (heightmap_reso == 0) heightmap_reso = 512;
#define IMG_SIZE heightmap_reso

			mMapOutParam = TsMapOutput(0, 0, 1, heightmap_reso, &mBoundingbox);
			TsUtil::SetDirectory("Resources/World/Landscape/Surface/");

			mSurfaces = TMap<EBiomeSType, TsBiomeSurface>{
				{ EBiomeSType::EBSf_None,
					TsBiomeSurface({
							new TsSurfaceField(1.0f, -5.0f     ),
							new TsSurfacePondNoise( TsNoiseParam({{0.8f,0.01f},{0.2f,0.03f}}), 0.5f, 0.1f),
						},
						new TsBiomeMFunc(
							EBiomeMapType::E_Moist,{
								{ 1.00f,0.0f, EMaterialType::EBMt_None  },
							})
					)
				},
				{ EBiomeSType::EBSf_Lake,
					TsBiomeSurface({
						new TsSurfaceLake(TsNoiseParam({{1.0f, 0.001f},{ 0.2f, 0.003f}}), -4.0f),
					},
						new TsBiomeMFunc(
							EBiomeMapType::E_Moist,{
								{ 0.60f, 0.0f, EMaterialType::EBMt_Soil_A },
								{ 0.50f, 0.0f, EMaterialType::EBMt_Soil_B },
							})
					)
				},
				{ EBiomeSType::EBSf_Field,
					TsBiomeSurface({
							new TsSurfaceField	  (1.00f, 5.0f     ),
							new TsSurfacePondNoise(TsNoiseParam({{0.40f,0.01f},{0.15f,0.03f}}), 0.5f, 0.1f),
						},
						new TsBiomeMFunc(
							EBiomeMapType::E_Moist,{
								{ 0.05f, 0.0f, EMaterialType::EBMt_Soil_A   },
								{ 0.10f, 0.0f, EMaterialType::EBMt_Soil_B   },
								{ 0.10f, 0.0f, EMaterialType::EBMt_Grass_A  },
								{ 0.35f, 0.0f, EMaterialType::EBMt_Grass_B  },
								{ 0.10f, 0.0f, EMaterialType::EBMt_Forest_A },
								{ 0.15f, 0.0f, EMaterialType::EBMt_Forest_B },
							})
					)
				},
				{ EBiomeSType::EBSf_Mountain,
					TsBiomeSurface({
							new TsSurfaceMountain ( 20       ),
							//new TsSurfacePondNoise(TsNoiseParam({{0.8f , 0.010f},{ 0.2f, 0.030f}}), 0.5f, 0.5f),
							//new TsSurfaceNoise    (10.0f, 2.0f, 1.4f),
						},
						new TsBiomeMFunc(
							EBiomeMapType::E_Moist,{
								{  0.20f, 0.0f, EMaterialType::EBMt_Grass_A },
								{  0.35f, 0.0f, EMaterialType::EBMt_Grass_B },
								{  0.10f, 0.0f, EMaterialType::EBMt_Soil_B  },
								{  0.45f, 0.0f, EMaterialType::EBMt_Soil_A  },
							})
					)
				},
			} ;

			////// apply the biome from PerlinNoise
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Apply Surface ..."));

			TsBiomeMap* surfc_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam({ {1.0f, 0.0010f},{0.2f, 0.0030f} }));
			TsBiomeMap* genre_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam({ {1.0f, 0.0010f},{0.2f, 0.0030f} }));

#if 0   ///later
			{
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Genre map ...") );

				TArray<TsBiomeSite> sites ;
				for ( const auto & c  : mShape.FindCircle( 0 ) ){// Level 0 meas the leaf of the continent...
					TsBiomeSite s ;
					s.Generate( c, radius, {
							TsBiomeSite::CircleConf(1.0f,1.0f, 0.06f,0.25f,6,-40,40),
							TsBiomeSite::CircleConf(0.4f,0.6f, 0.1f,0.3f,0,-40,40),
						} );
					sites.Add( s ) ;
				}

				genre_map->ForeachPixel(
					[&](int px, int py) {
						FVector2D p = genre_map->GetWorldPos(px, py);
						if (genre_map->IsWorld(p)) {
							int lvl = sites.Num() ;
							for ( const auto & s : sites ){
								if ( s.IsInside(p) ) break ;
								lvl-- ;
							}
							genre_map->SetPixel( px, py, lvl ) ;
						}
					} );
				genre_map->Save("GenreMap.dds", EImageFile::Dds, EImageFormat::FormatL16);

				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Genre map done.") );
			}
#endif

			// lock all of texturemaps
			//for (auto& tm : texture_maps) tm.Value->Lock();


			const float S = 0.5f ;
			TsMoistureMap* moist_map =
				new TsMoistureMap(
					IMG_SIZE, IMG_SIZE,
					&mBoundingbox, 
					TsNoiseParam({
						{ 0.90f, 0.001f*S },
						{ 0.50f, 0.002f*S },
						{ 0.25f, 0.004f*S },
						{ 0.10f, 0.016f*S },
						{ 0.10f, 0.032f*S },
						{ 0.10f, 0.064f*S },
					}),
					{
						//{ texture_maps[ETextureMap::ETM_Height   ],	 1.0f, EExtraOp::E_InvMul },
						//{ texture_maps[ETextureMap::ETM_Flow     ],	-1.2f, EExtraOp::E_Add },
						//{ texture_maps[ETextureMap::ETM_Slope    ],	-0.8f, EExtraOp::E_Add },
						//{ texture_maps[ETextureMap::ETM_Curvature],	-0.8f, EExtraOp::E_Add },
					}
				);
			{///---------------------------------------- Setup Biome
				// world once
				TArray<TsBiomeItem_SType> surfc_items = {
					{ 0.05f, 0.0f, EBiomeSType::EBSf_Lake    },
					{ 0.70f, 0.0f, EBiomeSType::EBSf_Field   },
					{ 0.25f, 0.0f, EBiomeSType::EBSf_Mountain},
				};
				TArray<TsBiomeItem_MType> moist_items = {
					{ 0.10f, 0.0f, EBiomeMType::EBMo_Soil	  },
					{ 0.35f, 0.0f, EBiomeMType::EBMo_Field	  },
					{ 0.05f, 0.0f, EBiomeMType::EBMo_Tree	  },
					{ 0.30f, 0.0f, EBiomeMType::EBMo_ForestA },
					{ 0.15f, 0.0f, EBiomeMType::EBMo_ForestB },
				};

				surfc_map->SetupItems< TsBiome, TsBiomeItem_SType >(mBiomes, surfc_items);
				moist_map->SetupItems< TsBiome, TsBiomeItem_MType >(mBiomes, moist_items);
				
				for ( auto& b : mBiomes ) {
					if (mShape.IsInside(b)) {
						b.SetSType( surfc_map->SelectItem<TsBiome, TsBiomeItem_SType>( b, surfc_items).mItem );
						b.SetMType( moist_map->SelectItem<TsBiome, TsBiomeItem_MType>( b, moist_items).mItem );
					}
				}

				surfc_map->ForeachPixel([&](int px, int py) {
						surfc_map->SetPixel(px, py,
							surfc_map->GetValue(surfc_map->GetWorldPos(px, py)));
					});
				surfc_map->Save("BM_SurfcMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
				surfc_map->Save("BM_SurfcMap.raw", EImageFile::Raw, EImageFormat::FormatL16);
			}

			// surface service initialize 
			TsSurfaceMountain::Initialize( mBiomes ) ;

#if 0//////////////////////////////////////////////////////////////////////////////////////// Integration of Generated Boime -----------bad result...
			{// Biome Specs
				TsBiomeGroup::ClearDone() ;
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Biome Specs ..."));

				FTsBiomeModels	biome_models(biome_specs) ;
				biome_models.Lock() ;

				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Biome Group ..."));
				// Grouping the Biomes.
				for ( auto &b : mBiomes ){
					if ( b.mSType != EBiomeSType::EBSf_None ){
						if ( !TsBiomeGroup::TryDone( &b ) ){
							mBiomeGroups.Add( TsBiomeGroup( &b, biome_models ) ) ;
						}
					}
				}

				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Construct Heightmap..."));
				// Generate heightmap.
				int				reso  = mMapOutParam.mWorldReso;
				const FBox2D*	bound = mMapOutParam.mWorldBound;
				mHeightMap   = new TsHeightMap  ( reso, reso, bound );
				mMaterialMap = new TsMaterialMap( reso, reso, bound,{
						EMaterialType::EBMt_None,
						EMaterialType::EBMt_Soil_A,
						EMaterialType::EBMt_Soil_B,
						EMaterialType::EBMt_Soil_C,
						EMaterialType::EBMt_Grass_A,
						EMaterialType::EBMt_Grass_B,
						EMaterialType::EBMt_Forest_A,
						EMaterialType::EBMt_Forest_B,
						EMaterialType::EBMt_Rock_A,
						EMaterialType::EBMt_Moss_A,
						EMaterialType::EBMt_Moss_B,
					});
				//TsBiomeMap* surf_map = new TsBiomeMap( reso, reso, &bound );
				//surf_map->ForeachPixel(
				//	[&](int px, int py) {	/////////// This is for debug use.
				//		FVector2D p = surf_map->GetWorldPos(px, py);
				//		if ( surf_map->IsWorld(p) ) {
				//			if (TsBiome* b = SearchBiome(p)) {
				//				surf_map->SetPixel(px, py, (float)b->GetSType() );
				//			}
				//		}
				//	} );
				//surf_map->Save("SurfMap.dds", EImageFile::Dds, EImageFormat::FormatL16 );

				mHeightMap->ForeachPixel(
					[&](int px, int py) {
						FVector2D p = mHeightMap->GetWorldPos(px, py);
						if ( mHeightMap->IsWorld(p) ) {
							if (TsBiomeGroup* g = SearchBiomeGroup(p)) {
								float h = g->GetPixel( ETextureMap::ETM_Height, p );

								UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: [%d,%d] %f"), px, py, h );

								mHeightMap->SetPixel(px, py, h );
							}
						}
					} );
				mHeightMap->Save("HeightMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
				mHeightMap->Save("HeightMap.raw", EImageFile::Raw, EImageFormat::FormatL16);

				//TsBiomeMap* surf_map = new TsBiomeMap( reso, reso, &bound );
				//surf_map->ForeachPixel(
				//	[&](int px, int py) {	/////////// This is for debug use.
				//		FVector2D p = surf_map->GetWorldPos(px, py);
				//		if ( surf_map->IsWorld(p) ) {
				//			if (TsBiomeGroup* g = SearchBiomeGroup(p)) {
				//				surf_map->SetPixel(px, py, (float)g->GetSeqID() );
				//			}
				//		}
				//	} );
				//surf_map->Save("SurfMap.dds", EImageFile::Dds, EImageFormat::FormatL16 );

				biome_models.UnLock() ;

				return ;//stop
			}
#endif

			{///---------------------------------------- Generate Mapping
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Generate Mappings ..."));
				int				reso  = mMapOutParam.mWorldReso;
				const FBox2D*	bound = mMapOutParam.mWorldBound;

				reso = 64;
//				reso = 4096;
				mHeightMap           = new TsHeightMap( reso, reso, bound );
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
										if (b->GetSType() == EBiomeSType::EBSf_Mountain || b->GetSType() == EBiomeSType::EBSf_Lake) {
											mSurfaces[EBiomeSType::EBSf_Field].UpdateRemap(b, p);
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
									if (b->GetSType() == EBiomeSType::EBSf_Mountain || b->GetSType() == EBiomeSType::EBSf_Lake) {
										h += mSurfaces[EBiomeSType::EBSf_Field].GetHeight(b, p);
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
							TsBiomeMap* flow_map = new TsBiomeMap ( reso, reso, bound );
							TsBiomeMap* pond_map = new TsBiomeMap ( reso, reso, bound );// disposition@‘ÍÏ

							TsErosion(mHeightMap, flow_map, pond_map).Simulate(erode_cycle);

							flow_map ->Save("BM_FlowMap.dds" , EImageFile::Dds, EImageFormat::FormatL16);
							flow_map ->Save("BM_FlowMap.raw" , EImageFile::Raw, EImageFormat::FormatL16);
							pond_map ->Save("BM_PondMap.dds" , EImageFile::Dds, EImageFormat::FormatL16);
							pond_map ->Save("BM_PondMap.raw" , EImageFile::Raw, EImageFormat::FormatL16);
						} else{
							UE_LOG(LogTemp, Log, TEXT("       skipped..."));
						}

						UE_LOG(LogTemp, Log, TEXT("   Erosion done."));
					}

					mHeightMap->Save("HeightMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
					mHeightMap->Save("HeightMap.raw", EImageFile::Raw, EImageFormat::FormatL16);

					//{///--------------------------------------------------- Normal
					//	UE_LOG(LogTemp, Log, TEXT("   Normal start..."));
					//	mHeightMap->ForeachPixel(
					//		[&](int px, int py) {
					//			mNormalMap->SetPixel(px, py, mHeightMap->CalcNormal(px, py, 4));
					//		});
					//	UE_LOG(LogTemp, Log, TEXT("   Normal done."));
					//}
					//mNormalMap->Save("NormalMap.dds", EImageFile::Dds, EImageFormat::FormatB8G8R8A8);


					UE_LOG(LogTemp, Log, TEXT("   HeightMap done."));
				}
#endif 



#if 0			/////////////////////////			material output will be this
				{// Layered Material			for Landscape(UE5) based material texture output.
					UE_LOG(LogTemp, Log, TEXT("BiomeSystem:: MaterialMap preparing..."));

					mMaterialMap         = new TsMaterialMap( reso, reso, &bound,{
							EMaterialType::EBMt_None,
							EMaterialType::EBMt_Soil_A,
							EMaterialType::EBMt_Soil_B,
							EMaterialType::EBMt_Soil_C,
							EMaterialType::EBMt_Grass_A,
							EMaterialType::EBMt_Grass_B,
							EMaterialType::EBMt_Forest_A,
							EMaterialType::EBMt_Forest_B,
							EMaterialType::EBMt_Rock_A,
							EMaterialType::EBMt_Moss_A,
							EMaterialType::EBMt_Moss_B,
						});

					TArray<TsBiomeItem_Material> moist_items = {
						{ 0.04f, 0.0f, EBMt_Soil_A,  },
						{ 0.02f, 0.0f, EBMt_Soil_B,  },
						{ 0.02f, 0.0f, EBMt_Soil_C,  },
						{ 0.10f, 0.0f, EBMt_Grass_A, },
						{ 0.35f, 0.0f, EBMt_Grass_B, },
						{ 0.20f, 0.0f, EBMt_Forest_A,},
						{ 0.15f, 0.0f, EBMt_Forest_B,},
					};
					bool first = true;
					moist_map->SetupItemsPixel< TsBiomeItem_Material >( moist_items );
					moist_map->ForeachPixel(
						[&](int px, int py) {
							TsMaterialPixel	pix;

							FVector2D p = moist_map->GetWorldPos(px, py);
							if (moist_map->IsWorld(p)) {
								TsUtil::ForeachGaussian(p, moist_map->GetStep(),
									[&](const FVector2D& pos, float weight) {
										int idx = moist_map->SelectItemIdx<FVector2D, TsBiomeItem_Material>(pos, moist_items);
										pix.Add(moist_items[idx].mItem, weight);
									}); 
							}

							mMaterialMap->MergePixel(px, py, pix);
						});
					UE_LOG(LogTemp, Log, TEXT("BiomeSystem:: MaterialMap prepare done."));

					UE_LOG(LogTemp, Log, TEXT("BiomeSystem:: MaterialMap exporting..."));
					mMaterialMap->StoreMaterial();
					mMaterialMap->SaveAll(0, 0, 0, 0);
					UE_LOG(LogTemp, Log, TEXT("BiomeSystem:: MaterialMap done."));
				}
#endif	// Layered Material

#if 0	// Material_map
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
							case EBiomeSType::EBSf_Lake:		return EMaterialType::MT_Sand_A; 
							case EBiomeSType::EBSf_Field:		return EMaterialType::MT_Grass_A; 
							case EBiomeSType::EBSf_Mountain:	return EMaterialType::MT_Forest_A;
							case EBiomeSType::EBSf_None:		return EMaterialType::MT_BaseLand;
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


					{///--------------------------------------------------- File Exporting 
						UE_LOG(LogTemp, Log, TEXT("    File exporting ..."));

						moist_map->ForeachPixel([&](int px, int py) {
								moist_map->SetPixel( px, py,
									moist_map->GetValue(moist_map->GetWorldPos(px, py)));
							});

						moist_map->Save("BM_MoistMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
						moist_map->Save("BM_MoistMap.raw", EImageFile::Raw, EImageFormat::FormatL16);

						UE_LOG(LogTemp, Log, TEXT("    File exporting done."));
					}
					UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Generate Mappings  done."));

				}
#endif		//material_map

			}


			{	///--------------------------------------------------- Generating  Meshes & Textures
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Grid-Resource  generate start..."));

				int				reso  = mMapOutParam.mWorldReso;
				const FBox2D*	bound = mMapOutParam.mWorldBound;
				mMaterialMap = new TsMaterialMap( reso, reso, bound,{
						EMaterialType::EBMt_None,
						EMaterialType::EBMt_Soil_A,
						EMaterialType::EBMt_Soil_B,
						EMaterialType::EBMt_Soil_C,
						EMaterialType::EBMt_Grass_A,
						EMaterialType::EBMt_Grass_B,
						EMaterialType::EBMt_Forest_A,
						EMaterialType::EBMt_Forest_B,
						EMaterialType::EBMt_Rock_A,
						EMaterialType::EBMt_Moss_A,
						EMaterialType::EBMt_Moss_B,
					});
				TArray<TsBiomeItem_Material> moist_items = {
					{ 0.04f, 0.0f, EBMt_Soil_A,  },
					{ 0.02f, 0.0f, EBMt_Soil_B,  },
					{ 0.02f, 0.0f, EBMt_Soil_C,  },
					{ 0.10f, 0.0f, EBMt_Grass_A, },
					{ 0.35f, 0.0f, EBMt_Grass_B, },
					{ 0.20f, 0.0f, EBMt_Forest_A,},
					{ 0.15f, 0.0f, EBMt_Forest_B,},
				};
				bool first = true;
				moist_map->SetupItemsPixel< TsBiomeItem_Material >( moist_items );

				TsTextureMap height_map( texture_maps[ETextureMap::ETM_Height] ) ;

				const int NN = 5 ;
				TsMapOutput	outparam( 24, 24, NN, height_map.GetSizeX(), &mBoundingbox );// 200pix * NN
				for ( int oy=0 ; oy<NN ; oy++ ){
					for ( int ox=0 ; ox<NN ; ox++ ){
						UE_LOG(LogTemp, Log, TEXT("    File Grid(%d %d) ..."), ox, oy );
						int		loc_reso	= 400 ;
						FBox2D	loc_bound	= outparam.LocalBound( ox, oy,loc_reso );

						TsUtil::SetDirectory( "Resources/World/Landscape/Surface/", ox, oy);

						mMaterialMap->Clear();
						mMaterialMap->SetWorld( &loc_bound );
						mMaterialMap->ForeachPixel([&](int px, int py) {
							TsMaterialPixel	pix;

							FVector2D p = mMaterialMap->GetWorldPos(px, py) ;
							if (moist_map->IsWorld(p)) {
								TsUtil::ForeachGaussian(p, moist_map->GetStep(),
									[&](const FVector2D& pos, float weight) {
										int idx = moist_map->SelectItemIdx<FVector2D, TsBiomeItem_Material>(pos, moist_items);
										pix.Add(moist_items[idx].mItem, weight);
									}); 
							}

							mMaterialMap->MergePixel(px, py, pix);
						});
						mMaterialMap->StoreMaterial();
						mMaterialMap->SaveAll(0,0,0,0);

						//TsMaterial::Build( 
						//		FString( "Resources/World/Landscape/Surface/" ),
						//		FString( "/Game/Resources/World/Landscape/Materials/M_Landscape.M_Landscape"),
						//		FString::Printf(TEXT("Materials/MT_Surface_%1d%1d"), ox, oy)
						//	) ;

						TsHeightMesh::Build(
								&height_map, 
								outparam.TexBound(ox,oy,loc_reso),	//TsUtil::TsBox(0,0,1024,1024)
								100,//1000,
								20000.0f, 10000.0f,
								FString::Printf(TEXT("SM_Surface_%1d%1d"), ox, oy)
							);
						UE_LOG(LogTemp, Log, TEXT("    File exporting done."));
					}
				}
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Grid-Resource  done."));
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

	//TMap<ETextureMap, TsTextureMap*> texture_maps;
	//for (auto& tm : mTextureMaps ) {
	//	texture_maps.Add(tm.Key, new TsTextureMap(tm.Value));
	//}

	work->BuildLandscape(
		pos.X, pos.Y, mRadius,
		123789,
		mVoronoiSize, mVoronoiJitter,
		mReso,
		mErodeCycle,

		mUnitSize, mUnitDiv, mUnitReso,

		mTextureMaps,

		mBiomeTable
	);

	work->Debug( GetWorld());

#endif	//WITH_EDITOR

}

