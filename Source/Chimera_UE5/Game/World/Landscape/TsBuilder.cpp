
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

	TArray<TsBiome>					mBiomes;

	TsBiomeSite*					mShape;

	FBox2D							mBoundingbox;
	float							mWaterLevel;

	TsMapOutput						mMapOutParam;

	TsHeightMap*					mHeightMap;
	TsHeightMap*					mNormalMap;
	TsMaterialMap*					mMaterialMap;

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
//		SurfaceMountain::Debug(world);

		mShape->Debug(world);

		for (auto b : mBiomes) {
			FColor c(0, 0, 0);
			switch (b.GetSType()) {
			case EBiomeSType::E_SurfNone:		c = FColor(0, 0, 180); break;
			case EBiomeSType::E_SurfLake:		c = FColor(100, 120, 255); break;
			case EBiomeSType::E_SurfField:		c = FColor(125, 255, 0); break;
			case EBiomeSType::E_SurfMountain:	c = FColor(120, 60, 20); break;
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
		float	voronoi_size,
		float	voronoi_jitter,
		int		heightmap_reso,
		int		erode_cycle)
	{
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
		}

		{///-------------------------------------------------------------------------------- Biome Group 		// you can access by 2d
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Biome Group"));
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: BiomeMap creating ..."));

			heightmap_reso = 64;
			if (heightmap_reso == 0) heightmap_reso = 512;
#define IMG_SIZE heightmap_reso

			mMapOutParam = TsMapOutput(0, 0, heightmap_reso, 1);
			TsImageCore::SetDirectory("Resources\\World\\Landscape\\Surface\\", mMapOutParam.x, mMapOutParam.y);

			mSurfaces = TMap<EBiomeSType, TsBiomeSurface>{
				{ EBiomeSType::E_SurfNone,
					TsBiomeSurface(
						TsBiomeSurface::Flag::FL_None,
						mMapOutParam.reso, mBoundingbox,
						"Masks\\MaskOcean.dds",
						-1,
						{	new SurfaceField(TsNoiseParam(1.0f,0    ,0   ,0), 20.0f),
							new SurfacePondNoise(TsNoiseParam(0.8f,0.01f,0.2f,0.03f), 0.5f, 0.4f),
						},
						{}
					)
				},
				{ EBiomeSType::E_SurfField,
					TsBiomeSurface(
						TsBiomeSurface::Flag::FL_None,
						IMG_SIZE, mBoundingbox,
						"Masks\\MaskField.dds",
						-1,
						{	new SurfaceField(TsNoiseParam(1.0f,0    ,0   ,0), 20.0f),
							new SurfacePondNoise(TsNoiseParam(0.8f,0.01f,0.2f,0.03f), 0.5f, 0.4f),
						},
						{}
					)
				},
				{ EBiomeSType::E_SurfLake,
					TsBiomeSurface(
						TsBiomeSurface::Flag::FL_None,
						IMG_SIZE, mBoundingbox,
						"Masks\\MaskLake.dds",
						-1,
						{	new SurfaceLake(	TsNoiseParam(1.0f, 0.0010f, 0.2f, 0.0030f), -10.0f	),
						},
						{}
					)
				},
				{ EBiomeSType::E_SurfMountain,
					TsBiomeSurface(
						TsBiomeSurface::Flag::FL_None,
						IMG_SIZE, mBoundingbox,
						"Masks\\MaskMount.dds",
						18,
						{	new SurfaceMountain ( TsNoiseParam(1.0f , 0.001f, 0.2f, 0.003f), 20        ),
							new SurfacePondNoise( TsNoiseParam(0.8f , 0.010f, 0.2f, 0.030f), 0.5f, 0.5f),
							new SurfaceNoise    ( TsNoiseParam(10.0f, 0     , 0   , 0     ), 2.0f, 1.4f),
						},
						{}
					)
				},
			} ;
			TMap<EBiomeSType, TsBiomeGroup>	surf_biomes = {
				{ EBiomeSType::E_SurfNone    , TsBiomeGroup{} },
				{ EBiomeSType::E_SurfLake    , TsBiomeGroup{} },
				{ EBiomeSType::E_SurfField   , TsBiomeGroup{} },
				{ EBiomeSType::E_SurfMountain, TsBiomeGroup{} },
			};

			////// apply the biome from PerlinNoise
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Apply Surface ..."));

			// world once
			TArray<TsBiomeItem_SType> surfs_items = {
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

			TsBiomeMap* surfs_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.0010f, 0.2f, 0.0030f));
			surfs_map->SetupItems<TsBiome, TsBiomeItem_SType>( mBiomes, surfs_items);
			TsBiomeMap* moist_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.0010f, 0.2f, 0.0030f));
			moist_map->SetupItems<TsBiome, TsBiomeItem_MType>( mBiomes, moist_items);

			UE_LOG(LogTemp, Log, TEXT("Lake %f"), surfs_items[0].mThreshold);
			UE_LOG(LogTemp, Log, TEXT("Fild %f"), surfs_items[1].mThreshold);
			UE_LOG(LogTemp, Log, TEXT("Mont %f"), surfs_items[2].mThreshold);

			for (auto& b : mBiomes) {
				if (mShape->GetValue(b) > 0.99999f) {
					surfs_map->SelectItem<TsBiome, TsBiomeItem_SType>(b, surfs_items,
						[&](const TsBiomeItem_SType& it) {
							b.SetSType( it.mItem );
						});
				}
			}

			{///---------------------------------------- Generate Mapping
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: HeightMap exporting..."));
				int		reso  = mMapOutParam.LocalReso();
				FBox2D	bound = mMapOutParam.LocalBound(mBoundingbox);

				mHeightMap   = new TsHeightMap  (reso, reso, &bound);
				mMaterialMap = new TsMaterialMap(reso, reso, &bound);

				{///---------------------------------------- HeightMap
					//mHeightMap->ForeachPixel([&](int px, int py) {
					//		FVector2D p = mHeightMap->GetWorldPos(px, py);
					//		if (mHeightMap->IsWorld(p)) {
					//			UpdateRemap(p);
					//		}
					//	});
					mHeightMap->ForeachPixel([&](int px, int py) {
							FVector2D p = mHeightMap->GetWorldPos(px, py);
							TsBiome*  b = SearchBiome(p);
							if ( b ) {
								mHeightMap->SetPixel(px, py, mSurfaces[b->GetSType()].GetHeight(b,p));
								UE_LOG(LogTemp, Log, TEXT("[%d,%d](%d) %f"), (int)px, (int)py, b->GetSType(), mSurfaces[b->GetSType()].GetHeight(b, p));
							}
						});
					UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: HeightMap done."));

					int out_reso = mMapOutParam.reso;
					int out_x    = mMapOutParam.LocalX();
					int out_y    = mMapOutParam.LocalY();
					mHeightMap->SaveAll(out_x, out_y, out_reso, out_reso);
				}
			}


#if 0
			{///---------------------------------------- HeightMap
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: HeightMap exporting..."));
				mHeightMap->ForeachPixel([&](int px, int py) {
					FVector2D p = mHeightMap->GetWorldPos(px, py);
					if (mHeightMap->IsWorld(p)) {
						UpdateRemap(p);
					}
					});
				mHeightMap->ForeachPixel([&](int px, int py) {
					FVector2D p = mHeightMap->GetWorldPos(px, py);
					mHeightMap->SetPixel(px, py, GetHeight(p));
					});
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: HeightMap done."));
			}

			{///--------------------------------------------------- Erosion
				TArray<TsVoronoi> v_list;
				SurfaceMountain::GetMountains(v_list);
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: creating Erosion  mountinas %d ..."), v_list.Num());

				TsErosion(mHeightMap, flow_map, pond_map).Simulate(erode_cycle);

				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Erosion done."));
			}

			{///--------------------------------------------------- Generate MaterialMap
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: MaterialMap preparing..."));

				// update Occupancy of the config...
				TsBiomeMatFunc::UpdateOccupancy();

				mMaterialMap->ForeachPixel([&](int px, int py) {
					FVector2D p = mMaterialMap->GetWorldPos(px, py);
					if (mMaterialMap->IsWorld(p)) {
						mMaterialMap->SetMaterial(px, py, GetMaterial(p));
					}
					});
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: MaterialMap prepare done."));

				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: MaterialMap exporting..."));
				mMaterialMap->ForeachPixel([&](int px, int py) {
					FVector2D p = mMaterialMap->GetWorldPos(px, py);
					if (mMaterialMap->IsWorld(p)) {
						mMaterialMap->SetMaterialPixel(px, py);
					}
					});
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: MaterialMap done."));

				UE_LOG(LogTemp, Log, TEXT("flow_map min=%f max=%f"), flow_map->mMin, flow_map->mMax);
			}

			int out_reso = mMapOutParam.reso;
			int out_x    = mMapOutParam.LocalX();
			int out_y    = mMapOutParam.LocalY();
			mHeightMap  ->SaveAll(out_x, out_y, out_reso, out_reso);
			mMaterialMap->SaveAll(out_x, out_y, out_reso, out_reso);

			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: FileOut %d %d reso%d "), out_x, out_y, out_reso);
#endif

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
		mVoronoiSize, mVoronoiJitter,
		mReso,
		mErodeCycle
	);

	work->Debug( GetWorld());

#endif	//WITH_EDITOR

}

