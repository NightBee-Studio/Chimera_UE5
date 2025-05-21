
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

			heightmap_reso = 1024 ;
			if (heightmap_reso == 0) heightmap_reso = 512;
#define IMG_SIZE heightmap_reso

			mMapOutParam = TsMapOutput(0, 0, heightmap_reso, 1);
			TsImageCore::SetDirectory("Resources\\World\\Landscape\\Surface\\", mMapOutParam.x, mMapOutParam.y);

			mSurfaces = TMap<EBiomeSType, TsBiomeSurface>{
				{ EBiomeSType::E_SurfNone,
					TsBiomeSurface({
							new TsSurfaceField	  (TsNoiseParam(1.0f,0    ,0   ,0    ), 5.0f     ),
							new TsSurfacePondNoise(TsNoiseParam(0.8f,0.01f,0.2f,0.03f), 0.5f, 0.4f),
						},{
						})
				},
				{ EBiomeSType::E_SurfField,
					TsBiomeSurface({
							new TsSurfaceField	  (TsNoiseParam(1.00f,0    ,0    ,0    ), 10.0f     ),
							new TsSurfacePondNoise(TsNoiseParam(0.40f,0.01f,0.15f,0.03f), 0.5f, 0.4f),
						},{
						})
				},
				{ EBiomeSType::E_SurfLake,
					TsBiomeSurface({
							new TsSurfaceLake	  (TsNoiseParam(1.0f, 0.001f, 0.2f, 0.003f), -10.0f	),
						},{
							new TsBiomeMFunc({
								{	EBiomeMapType::E_Plant,
									{	{ EMaterialType::MT_LakeSoil_A , 1.0f, nullptr, },
									}
								}
							}),
						})
				},
				{ EBiomeSType::E_SurfMountain,
					TsBiomeSurface({
							new TsSurfaceMountain (TsNoiseParam(1.0f , 0.001f, 0.2f, 0.003f), 20       ),
							//new TsSurfacePondNoise(TsNoiseParam(0.8f , 0.010f, 0.2f, 0.030f), 0.5f, 0.5f),
							//new TsSurfaceNoise    (TsNoiseParam(10.0f, 0     , 0   , 0     ), 2.0f, 1.4f),
						},{
							new TsBiomeMFunc({
								{	EBiomeMapType::E_Mountain,
									{	{ EMaterialType::MT_Grass_A, 0.50f, nullptr, },//new Op_PixelLt(BiomeMapType::E_Flow,0.3f)
										{ EMaterialType::MT_Grass_B, 0.35f, nullptr, },//new Op_PixelLt(BiomeMapType::E_Flow,0.3f)
										{ EMaterialType::MT_Soil_B , 0.10f, nullptr, },
										{ EMaterialType::MT_Soil_A , 0.05f, nullptr, },
									},
								},
								{	EBiomeMapType::E_Flow,
									{	{ EMaterialType::MT_Moss_B , 1.10f, nullptr, },
									},
								},
							}),
						})
				},
			} ;

			////// apply the biome from PerlinNoise
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Apply Surface ..."));

			TsBiomeMap* surfc_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.0010f, 0.2f, 0.0030f));
			TsBiomeMap* moist_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.0010f, 0.2f, 0.0030f));

			{///---------------------------------------- Setup Biome
				// world once
				TArray<TsBiomeItem_SType> surfc_items = {
					{ 0.05f, 0.0f, EBiomeSType::E_SurfLake    },
					{ 0.70f, 0.0f, EBiomeSType::E_SurfField   },
					{ 0.25f, 0.0f, EBiomeSType::E_SurfMountain},
				};
				TArray<TsBiomeItem_MType> moist_items = {
					{ 0.10f, 0.0f, EBiomeMType::E_Soil		},
					{ 0.50f, 0.0f, EBiomeMType::E_Field		},
					{ 0.05f, 0.0f, EBiomeMType::E_Tree		},
					{ 0.35f, 0.0f, EBiomeMType::E_Forest	},
				};

				surfc_map->SetupItems< TsBiome, TsBiomeItem_SType >(mBiomes, surfc_items);
				moist_map->SetupItems< TsBiome, TsBiomeItem_MType >(mBiomes, moist_items);
				for (auto& b : mBiomes) {
					if (mShape->IsInside(b)) {
						surfc_map->SelectItem<TsBiome, TsBiomeItem_SType>(
							b, surfc_items,
							[&](const TsBiomeItem_SType& it) {
								b.SetSType(it.mItem);
							}
						);
						moist_map->SelectItem<TsBiome, TsBiomeItem_MType>(
							b, moist_items,
							[&](const TsBiomeItem_MType& it) {
								b.SetMType(it.mItem);
							}
						);
					}
				}
			}

			{///---------------------------------------- Generate Mapping
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Generate Mappings ..."));
				int		reso  = mMapOutParam.LocalReso();
				FBox2D	bound = mMapOutParam.LocalBound(mBoundingbox);

				mHeightMap    = new TsHeightMap  (reso, reso, &bound);
				mNormalMap	  = new TsNormalMap  (reso, reso, &bound);
				mMaterialMap  = new TsMaterialMap(reso, reso, &bound);

				TsBiomeMap* norm_map = new TsBiomeMap(reso, reso, &bound);
				TsBiomeMap* flow_map = new TsBiomeMap(reso, reso, &bound);
				TsBiomeMap* pond_map = new TsBiomeMap(reso, reso, &bound);
				TsBiomeMap::AddBiomeMap(EBiomeMapType::E_Normal, norm_map);	//norm
				TsBiomeMap::AddBiomeMap(EBiomeMapType::E_Flow  , flow_map);	//flow
				TsBiomeMap::AddBiomeMap(EBiomeMapType::E_Pond  , pond_map);	//pond

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
//if (b->GetSType() == EBiomeSType::E_SurfMountain)
									mHeightMap->SetPixel(px, py, h);
								}
							});
						mHeightMap->Normalize();		// normalize to 0-1

						UE_LOG(LogTemp, Log, TEXT("   Base Heightmap done."));
					}

					{///--------------------------------------------------- Erosion
						UE_LOG(LogTemp, Log, TEXT("   Erosion start..."));

						erode_cycle = 1;
						TsErosion( mHeightMap, flow_map, pond_map ).Simulate( erode_cycle );

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

					mHeightMap->Save("HeightMap.dds", EImageFile::Dds, EImageFormat::FormatL16     );
					mNormalMap->Save("NormalMap.dds", EImageFile::Dds, EImageFormat::FormatB8G8R8A8);

					UE_LOG(LogTemp, Log, TEXT("    HeightMap done."));
				}

				{///--------------------------------------------------- MaterialMap

				}

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
					surfc_map->Save("BM_SurfcMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
					flow_map ->Save("BM_FlowMap.dds" , EImageFile::Dds, EImageFormat::FormatL16);
					pond_map ->Save("BM_PondMap.dds" , EImageFile::Dds, EImageFormat::FormatL16);

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

