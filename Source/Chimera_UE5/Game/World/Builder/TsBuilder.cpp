	
#include "TsBuilder.h"

#include "Biome/TsBiome.h"
#include "Biome/TsBiomeMap.h"
#include "Biome/TsBiomeModel.h"
#include "Biome/TsBiomeSite.h"
#include "Biome/TsBiomeSurface.h"

#include "Util/TsBuilderTool.h"
#include "Util/TsErosion.h"
#include "Util/TsVoronoi.h"
#include "Util/TsUtility.h"
#include "Util/TsMaskMap.h"

#include "Materials/MaterialInstanceConstant.h"



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
	TsMaskMap*							mMaskBiome;
	TsMaskMap*							mMaskTypeS;

private:
	TsBiome*		SearchBiome(const FVector2D& p, bool simple = true )		// world-coord
	{
		if ( simple ){
			for (auto& b : mBiomes) {
				if ( b.IsInside(p) ) return &b;
			}
		} else {
			for (auto& b : mBiomes) {
				if ( b.mEdges.Num() == 0) continue ;

				auto cross = [&](const TsVoronoi::Edge& e) {
					return e.mD.X * (p.Y - e.mP.Y) - e.mD.Y * (p.X - e.mP.X);
				} ;

				TsBiome	*	bio = nullptr ;
				float		msk = 1 ;

				auto checkedge = [&]() {
					float dd = 0;
					for ( auto e : b.mEdges ) {
						if ( e.mShared ){
							TsBiome	*a = (TsBiome*)e.mShared->mOwner ;
							if ( a->mSType < b.mSType ){
								float m = FMath::Clamp(e.GetDistance(p) / 100, 0.0f, 1.0f);
								if ( msk > m  ) { msk = m; bio = a ;}
							}
						}
						if      (dd == 0.0f) dd = cross(e);
						else if (dd * cross(e) <= 0.0f) return false ;
					}
					return true ;
				} ;
				if ( checkedge()){////inside the voronoi
					if ( bio){
						float mp = mMaskBiome->Remap( mMaskBiome->GetValue(p)) ;
//						UE_LOG(LogTemp, Log, TEXT("  Found biome (%f,%f) mp %f < msk %f  "), bio->X, bio->Y, mp, 1-msk );
						if ( mp < 1-msk  ){
							return bio ;
						}
					}
					return &b;
				}
			}
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
			int			mode,
			float _x, float _y, float radius,
			int			seed,
			float		voronoi_size,
			float		voronoi_jitter,
			int			heightmap_reso,
			int			erode_cycle,

			int			unit_size,
			int			unit_div ,
			int			unit_reso,

			TMap<TEnumAsByte<EMaterialType>,FTsGroundTex>&			texture_sets,
			TMap<TEnumAsByte<ETextureMap>,TObjectPtr<UTexture2D>>&	texture_maps,

			UDataTable*	biome_specs
		)
	{
		mode = 0 ;
//		mode |= (int)EBuildMode::EBM_HeightMap   ;
		mode |= (int)EBuildMode::EBM_MaterialMap ;
//		mode |= (int)EBuildMode::EBM_FoliageMap	 ;
//		mode |= (int)EBuildMode::EBM_StaticMesh	 ;
		mode |= (int)EBuildMode::EBM_UpdateRatio ;

		TsUtil::RandSeed( seed );

		UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Build Start.... center(%f,%f) radius%f"), _x, _y, radius );

		//clean up first.
		Release();

		// create island shape
		mShape.Generate(_x, _y, radius,
			{TsBiomeSite::CircleConf(1.0f,1.5f, 0.06f,0.4f, 3, -40, 40),
			 TsBiomeSite::CircleConf(0.4f,0.8f, 0.06f,0.5f, 1,  20, 40),
//			 TsBiomeSite::CircleConf(0.6f,0.9f, 0.06f,0.5f, 1, -40,-20),
			 TsBiomeSite::CircleConf(0.8f,1.2f, 0.06f,0.5f, 0, -40,-20),} );

		{//-------------------------------------------------------------------------------------- create voronois
			mShape.UpdateBoundingbox(mBoundingbox);
			FVector2D size   = mBoundingbox.GetSize() ;
			FVector2D center = mBoundingbox.GetCenter() ;
			UE_LOG(LogTemp, Log, TEXT("mBoundingbox center(%f,%f) size(%f,%f)"), center.X,center.Y , size.X,size.Y  );

			float hx=300, hy=300 ;
			if ( size.X < size.Y ){// try to fix as same size...
				hx += (size.Y - size.X)*0.5f;
			} else {
				hy += (size.X - size.Y)*0.5f;
			}
			mBoundingbox.Min -= FVector2D(hx, hy);
			mBoundingbox.Max += FVector2D(hx, hy);
			mBoundingbox.Min -= center ;
			mBoundingbox.Max -= center ;

			UE_LOG(LogTemp, Log, TEXT("mBoundingbox size(%f,%f)"), mBoundingbox.GetSize().X,mBoundingbox.GetSize().Y  );


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

			heightmap_reso = 2048;
			if (heightmap_reso == 0) heightmap_reso = 512;
#define IMG_SIZE heightmap_reso


			////// apply the biome from PerlinNoise
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Apply Surface ..."));

			TsBiomeMap* genre_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam({ {1.0f, 0.0010f},{0.2f, 0.0030f} }));
			TsBiomeMap* surfc_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam({ {1.0f, 0.0010f},{0.2f, 0.0030f} }));


			mMapOutParam = TsMapOutput(0, 0, 1, heightmap_reso, &mBoundingbox);
			TsUtil::SetDirectory("Resources/World/Landscape/Surface/");

			mSurfaces = TMap<EBiomeSType, TsBiomeSurface>{
				{ EBiomeSType::EBSf_None,
					TsBiomeSurface({
							new TsSurfaceField(1.0f, -2.0f     ),
							new TsSurfacePondNoise( TsNoiseParam({{0.8f,0.01f},{0.2f,0.03f}}), 0.3f, 0.2f),
						},{
							new TsBiomeMFunc(
								ETextureMap::ETM_Moisture,{
									{ 0.30f, -0.133543f, EMaterialType::EBMt_Clay_A },
									{ 0.30f,  0.102713f, EMaterialType::EBMt_Soil_B },
									{ 0.30f,  0.391136f, EMaterialType::EBMt_Clay_D },
								},{
								}),
							new TsBiomeMFunc(
								ETextureMap::ETM_Slope,{
									{  1.0f, 0.0f, EMaterialType::EBMt_Mountain_E  },
								},{
									{ EBiomeParamType::EBPt_Scale, {.f=0.4f} }
								}),
							new TsBiomeMFunc(
								ETextureMap::ETM_Flow,{
									{  1.0f, 0.0f, EMaterialType::EBMt_Mountain_F  },
								},{
									{ EBiomeParamType::EBPt_Scale, {.f=1.5f} }
								}),
						})
				},
				{ EBiomeSType::EBSf_Lake,
					TsBiomeSurface({
							new TsSurfaceLake(TsNoiseParam({{1.0f, 0.001f},{ 0.2f, 0.003f}}), -4.0f),
						},{
							//new TsBiomeMFunc(
							//	ETextureMap::ETM_Moisture,{
							//		{ 0.60f, 0.577534f, EMaterialType::EBMt_Moss_A },
							//		{ 0.50f, 0.885273f, EMaterialType::EBMt_Moss_B },
							//	},{} ),
							new TsBiomeMFunc(
								ETextureMap::ETM_Slope,{
									{  1.0f, 0.0f, EMaterialType::EBMt_Mountain_E  },
								},{
									{ EBiomeParamType::EBPt_Scale, {.f=0.4f} }
								}),
							new TsBiomeMFunc(
								ETextureMap::ETM_Flow,{
									{  1.0f, 0.0f, EMaterialType::EBMt_Mountain_F  },
								},{
									{ EBiomeParamType::EBPt_Scale, {.f=1.5f} }
								}),
						})
				},
				{ EBiomeSType::EBSf_Field,
					TsBiomeSurface({
							new TsSurfaceField	  (1.00f, 3.0f     ),
							new TsSurfacePondNoise(TsNoiseParam({{0.40f,0.01f},{0.15f,0.03f}}), 0.5f, 0.1f),
						},{
							new TsBiomeMFunc(
								ETextureMap::ETM_Moisture,{
									{ 0.05f, 0.0f, EMaterialType::EBMt_Soil_A   },
									{ 0.08f, 0.0f, EMaterialType::EBMt_Soil_B   },
									{ 0.20f, 0.0f, EMaterialType::EBMt_Grass_A  },
									{ 0.255f,0.0f, EMaterialType::EBMt_Grass_B  },
									{ 0.005f,0.0f, EMaterialType::EBMt_Grass_C  },
									{ 0.15f, 0.0f, EMaterialType::EBMt_Forest_A },
									{ 0.25f, 0.0f, EMaterialType::EBMt_Forest_B },
								},{
								}),
							new TsBiomeMFunc(
								ETextureMap::ETM_Slope,{
									{  1.0f, 0.0f, EMaterialType::EBMt_Mountain_E  },
								},{
									{ EBiomeParamType::EBPt_Scale, {.f=0.4f} }
								}),
							new TsBiomeMFunc(
								ETextureMap::ETM_Flow,{
									{  1.0f, 0.0f, EMaterialType::EBMt_Mountain_F  },
								},{
									{ EBiomeParamType::EBPt_Scale, {.f=1.5f} }
								}),
						})
				},
				{ EBiomeSType::EBSf_Mountain,
					TsBiomeSurface({
							new TsSurfaceMountain ( 20       ),
							//new TsSurfacePondNoise(TsNoiseParam({{0.8f , 0.010f},{ 0.2f, 0.030f}}), 0.5f, 0.5f),
							//new TsSurfaceNoise    (10.0f, 2.0f, 1.4f),
						},{
							//new TsBiomeMFunc(
							//	ETextureMap::ETM_Moisture,{
							//		{  0.30f, -0.301768f, EMaterialType::EBMt_Mountain_A  },
							//		{  0.40f,  0.042033f, EMaterialType::EBMt_Mountain_B  },
							//		{  0.05f,  0.135508f, EMaterialType::EBMt_Mountain_C  },
							//		{  0.05f,  0.230635f, EMaterialType::EBMt_Mountain_D  },
							//	}),
							new TsBiomeMFunc(
								ETextureMap::ETM_Slope,{
									{  1.0f, 0.0f, EMaterialType::EBMt_Mountain_E  },
								},{
									{ EBiomeParamType::EBPt_Scale, {.f=0.4f} }
								}),
							new TsBiomeMFunc(
								ETextureMap::ETM_Flow,{
									{  1.0f, 0.0f, EMaterialType::EBMt_Mountain_F  },
								},{
									{ EBiomeParamType::EBPt_Scale, {.f=2.0f} }
								}),
						})
				},
			} ;

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


			const float S = 0.08f ;//corse <- S ->small
			TsMoistureMap* moist_map =
				new TsMoistureMap( &mBoundingbox, TsNoiseParam({{0.8f,0.009f*S},{0.4f,0.03f*S},{0.1f,0.64f*S} }),
					{
						TsExtraMap( ETextureMap::ETM_Height   , texture_maps[ETextureMap::ETM_Height   ], &mBoundingbox,	0.0f, EExtraOp::E_Add	),	//	 1.0f E_InvMul
						TsExtraMap( ETextureMap::ETM_Flow     , texture_maps[ETextureMap::ETM_Flow     ], &mBoundingbox,   -0.5f, EExtraOp::E_Add		),	//  -1.2f
						TsExtraMap( ETextureMap::ETM_Slope    , texture_maps[ETextureMap::ETM_Slope    ], &mBoundingbox,    1.5f, EExtraOp::E_InvMul),	//  -1.8f
						TsExtraMap( ETextureMap::ETM_Curvature, texture_maps[ETextureMap::ETM_Curvature], &mBoundingbox,	0.0f, EExtraOp::E_Add		),	//  -1.8f
					}
				);
			moist_map->Lock() ;// for read extra-maps
			moist_map->UpdateNoiseRemap();

			TsBiomeMap::AddBiomeMap( ETextureMap::ETM_Moisture , moist_map ) ;
			TsBiomeMap::AddBiomeMap( ETextureMap::ETM_Height   , &moist_map->mExtras[0]) ;
			TsBiomeMap::AddBiomeMap( ETextureMap::ETM_Flow     , &moist_map->mExtras[1]) ;
			TsBiomeMap::AddBiomeMap( ETextureMap::ETM_Slope    , &moist_map->mExtras[2]) ;
			TsBiomeMap::AddBiomeMap( ETextureMap::ETM_Curvature, &moist_map->mExtras[3]) ;

			{///---------------------------------------- Setup Biome
				// world once
				TArray<TsBiomeItem_SType> surfc_items = {
					{ 0.05f, 0.0f, EBiomeSType::EBSf_Lake    },
					{ 0.70f, 0.0f, EBiomeSType::EBSf_Field   },
					{ 0.25f, 0.0f, EBiomeSType::EBSf_Mountain},
				};

				surfc_map->SetupItems< TsBiome, TsBiomeItem_SType >(mBiomes, surfc_items);

				for ( auto& b : mBiomes ) {
					if (mShape.IsInside(b)) {
						b.SetSType( surfc_map->SelectItem<TsBiome, TsBiomeItem_SType>( b, surfc_items).mItem );
					}
				}
			}

			// surface service initialize 
			TsSurfaceMountain::Initialize( mBiomes ) ;

		
			///---------------------------------------- Generate Mapping
			{
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Generate Mappings ..."));
				int				reso  = mMapOutParam.mWorldReso;
				const FBox2D*	bound = mMapOutParam.mWorldBound;

				reso = 1024;
				mHeightMap           = new TsHeightMap( reso, reso, bound );

				///---------------------------------------- HeightMap
				if ( mode & (int)EBuildMode::EBM_HeightMap ){
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

							TsErosion( mHeightMap, flow_map, pond_map ).Simulate(erode_cycle);

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

			}


			{	///--------------------------------------------------- Generating  Meshes & Textures
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Grid-Resource  generate start..."));

				TArray<TsUtil::TsIPoint> points ;
				const int NN = 5 ;
				for ( int oy=0 ; oy<NN ; oy++ ){
					for ( int ox=0 ; ox<NN ; ox++ ) points.Add( TsUtil::TsIPoint(ox,oy) ) ;
				}
				//points.Add( TsUtil::TsIPoint(3,2) ) ;

				mMaskBiome = new TsMaskMap( 0.1f ) ;
				mMaskTypeS = new TsMaskMap( 0.4f ) ;

				int				reso  = mMapOutParam.mWorldReso;
				const FBox2D*	bound = mMapOutParam.mWorldBound;
				mMaterialMap = new TsMaterialMap( reso, reso, bound);
				if ( mode & (int)EBuildMode::EBM_UpdateRatio  ) {
					for ( auto s_type : {
							EBiomeSType::EBSf_None ,
							EBiomeSType::EBSf_Lake ,
							EBiomeSType::EBSf_Field,
							EBiomeSType::EBSf_Mountain } ){
						UE_LOG(LogTemp, Log, TEXT("    SetupItemsPixel(s_type %s) ..."), *StaticEnum<EBiomeSType>()->GetNameStringByValue(static_cast<int64>(s_type)) );

						for ( auto *m : mSurfaces[s_type].mMFuncs ){
							if  (m->mItems.Num() == 1 ){
								UE_LOG(LogTemp, Log, TEXT("         <%s> is skipped."),
									*StaticEnum<ETextureMap  >()->GetNameStringByValue(static_cast<int64>(m->mMapType)) ) ;
								continue ;
							}

							TsBiomeMap *	biomap = TsBiomeMap::GetBiomeMap(m->mMapType) ;
							biomap->SetupItemsPixel< TsBiomeItem_Material >(
								m->mItems,
								[&]( int px, int py )->bool{
									FVector2D p = biomap->GetWorldPos(px, py) ;
									if ( biomap->IsWorld( p )) {
										return SearchBiome( p, false )->mSType == s_type ;
									}
									return false ;
								} );

							for ( auto &it : m->mItems ){
								UE_LOG(LogTemp, Log, TEXT("         <%s> [%s] r%f t%f(%f)"),
									*StaticEnum<ETextureMap  >()->GetNameStringByValue(static_cast<int64>(m->mMapType)), 
									*StaticEnum<EMaterialType>()->GetNameStringByValue(static_cast<int64>(it.mItem)), it.mRatio, biomap->Remap(it.mThreshold), it.mThreshold );
							}
						}
						UE_LOG(LogTemp, Log, TEXT("    SetupItemsPixel  done ...") );
					}

					{
						TsBiomeMap *biomap = TsBiomeMap::GetBiomeMap(ETextureMap::ETM_Moisture) ;
						UE_LOG(LogTemp, Log, TEXT("    Moist MinMax(%f %f)"), biomap->mMin, biomap->mMax );
					}
				}

				TsBiomeMap	height_map( texture_maps[ETextureMap::ETM_Height] ) ;
				TsBiomeMap	foliage_map( reso, reso, bound ) ;

				const float NS = 1.0f ;
				TsNoiseMap		noise_map(
					TsNoiseParam({
						{ 0.10f,  8.0f*NS },
						{ 0.05f, 16.0f*NS },
						{ 0.25f, 32.0f*NS },
					}) ) ;
				int width = height_map.GetW();
				TsMapOutput	outparam( 0, 0, NN, width, &mBoundingbox );// 200pix * NN
				for ( auto &pn : points ){
					UE_LOG(LogTemp, Log, TEXT("    File Grid(%d %d) ...Bound(%f %f)-(%f %f)"), pn.mX, pn.mY, mBoundingbox.Min.X, mBoundingbox.Min.Y, mBoundingbox.Max.X, mBoundingbox.Max.Y );
					int		loc_reso	= (((int)(width/100)))*100/NN ;
					FBox2D	loc_bound	= outparam.LocalBound( pn.mX, pn.mY ,loc_reso );
					UMaterialInstanceConstant* surf_mat = nullptr ;

					TsUtil::SetDirectory( "Resources/World/Landscape/Surface/", pn.mX, pn.mY );
					foliage_map.ClearImage() ;
					foliage_map.SetWorld( &loc_bound );
					TArray<EMaterialType> foliage_items ;

					TsUtil::SetSubDirectory( TEXT("Materials/") );
					if ( mode & (int)EBuildMode::EBM_MaterialMap ){// materials
						UE_LOG(LogTemp, Log, TEXT("       Material start ...") );

						mMaterialMap->Clear();
						mMaterialMap->SetWorld( &loc_bound );
						mMaterialMap->ForeachPixel(
							[&](int px, int py) {
								TsMaterialPixel	pix;
								FVector2D		p = mMaterialMap->GetWorldPos(px, py) ;

								TsBiome *		biome  = SearchBiome( p, false );
								EBiomeSType		s_type = biome ? biome->mSType : EBiomeSType::EBSf_None ;
								for ( auto *m : mSurfaces[s_type].mMFuncs ){
									TArray<TsBiomeItem_Material>  &items = m->mItems ;
									TsBiomeMap *biomap = TsBiomeMap::GetBiomeMap(m->mMapType) ;

									if ( items.Num()==1 ){
										pix.Add( items[0].mItem, biomap->GetValue(p) * m->mParams[EBiomeParamType::EBPt_Scale].f );
									} else {
										TsBiomeMap::ItemResult r = biomap->SelectItemIdx  <FVector2D, TsBiomeItem_Material>(p, items) ;
										if ( items[r.mIndex].mItem > 0 ){
											float mp = mMaskTypeS->Remap( mMaskTypeS->GetValue(p)) ;
											EMaterialType it ; 
											if ( mp > r.mValue && r.mIndex > 0 ){
												it = items[r.mIndex-1].mItem ;
											} else {
												it = items[r.mIndex  ].mItem ;
											}
											pix.Add( it, 1.0f );

											if ( s_type==EBiomeSType::EBSf_Field ){
												foliage_map.SetPixel( px, py, 1-((float)it + r.mValue)/EBMt_Max );
												foliage_items.AddUnique( it ) ;
											}
										}
									}
								}
								mMaterialMap->MergePixel(px, py, pix);
							});
						mMaterialMap->SaveAll(0,0,0,0);
						surf_mat = TsBuilderTool::Build_MaterialInstance( 
								FString( "/Game/Resources/World/Landscape/Materials/M_Landscape.M_Landscape"),
								FString::Printf(TEXT("MT_Surface_%1d%1d"), pn.mX, pn.mY ),
								mMaterialMap->GetTexParams()
							) ;
						UE_LOG(LogTemp, Log, TEXT("       Material done.") );
					} else {
						FString asset_name = FString::Printf(TEXT("MT_Surface_%1d%1d"), pn.mX, pn.mY );
						FString package_path = TsUtil::GetPackagePath( asset_name + "." + asset_name ) ;
						surf_mat = LoadObject<UMaterialInstanceConstant>(nullptr, *package_path);
						UE_LOG(LogTemp, Log, TEXT("       Material loaded.") );
					}
					TsUtil::SetSubDirectory( TEXT("") );	//reset

					if ( foliage_items.Num() > 0  ){
						FString asset_name = FString::Printf(TEXT("T_FoliageMap_%1d%1d"), pn.mX, pn.mY );
						foliage_map.SaveAsset(*asset_name, EImageFormat::FormatR8 ) ;
						for ( auto &it : foliage_items ){
							UE_LOG(LogTemp, Log, TEXT("       Foriage[%s] %f -> %f"),
								*StaticEnum<EMaterialType>()->GetNameStringByValue(static_cast<int64>(it)), 1-((float)it)/EBMt_Max, 1-((float)it+1)/EBMt_Max );	
						}
					}

					if ( mode & (int)EBuildMode::EBM_StaticMesh ){
						UE_LOG(LogTemp, Log, TEXT("       StaticMesh start...") );
						TsBuilderTool::Build_HeightMesh(
								&height_map,
								outparam.TexBound(pn.mX, pn.mY ,loc_reso),	//TsUtil::TsBox(0,0,1024,1024)
								&noise_map,
								0.0005f,			// noise scale
								1000,		
								40000.0f, 14000.0f,
								FString::Printf(TEXT("SM_Surface_%1d%1d"), pn.mX, pn.mY ),
								static_cast<UMaterialInterface*>(surf_mat)
							);
								//0.0005f,			// noise scale
								//2000,		
								//20000.0f, 17000.0f,
						UE_LOG(LogTemp, Log, TEXT("       StaticMesh done.") );
					}
					UE_LOG(LogTemp, Log, TEXT("    File Grid(%d,%d) exporting done."), pn.mX, pn.mY );
				}
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Grid-Resource  done."));
				moist_map->UnLock() ;
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
		mMode,
		pos.X, pos.Y, mRadius,
		mSeed,
		mVoronoiSize, mVoronoiJitter,//0x8ac79,
		mReso,
		mErodeCycle,

		mUnitSize, mUnitDiv, mUnitReso,

		mMaterialSets,
		mTextureMaps,

		mBiomeTable
	);

	work->Debug( GetWorld());

#endif	//WITH_EDITOR

}

