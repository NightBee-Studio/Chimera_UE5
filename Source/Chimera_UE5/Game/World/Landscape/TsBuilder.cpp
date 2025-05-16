
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

		for (auto &b : mBiomes) {
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

			for (auto& b : mBiomes) {
				b.SetOwner();
			}
		}

		{///-------------------------------------------------------------------------------- Biome Group 		// you can access by 2d
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Biome Group"));
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: BiomeMap creating ..."));

			heightmap_reso = 512;
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
						} )
				},
				{ EBiomeSType::E_SurfField,
					TsBiomeSurface({
							new TsSurfaceField	  (TsNoiseParam(1.0f,0    ,0   ,0    ), 10.0f     ),
							new TsSurfacePondNoise(TsNoiseParam(0.8f,0.01f,0.2f,0.03f), 0.5f, 0.4f),
						},
						{})
				},
				{ EBiomeSType::E_SurfLake,
					TsBiomeSurface({
							new TsSurfaceLake	  (TsNoiseParam(1.0f, 0.001f, 0.2f, 0.003f), -10.0f	),
						},
						{})
				},
				{ EBiomeSType::E_SurfMountain,
					TsBiomeSurface({
							new TsSurfaceMountain (TsNoiseParam(1.0f , 0.001f, 0.2f, 0.003f), 40       ),
							new TsSurfacePondNoise(TsNoiseParam(0.8f , 0.010f, 0.2f, 0.030f), 0.5f, 0.5f),
							new TsSurfaceNoise    (TsNoiseParam(10.0f, 0     , 0   , 0     ), 2.0f, 1.4f),
						},{
						})
				},
			} ;

			////// apply the biome from PerlinNoise
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Apply Surface ..."));

			TsBiomeMap* surfs_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.0010f, 0.2f, 0.0030f));
			TsBiomeMap* moist_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.0010f, 0.2f, 0.0030f));

			{///---------------------------------------- Setup Biome
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

				surfs_map->SetupItems< TsBiome, TsBiomeItem_SType >(mBiomes, surfs_items);
				moist_map->SetupItems< TsBiome, TsBiomeItem_MType >(mBiomes, moist_items);
				for (auto& b : mBiomes) {
					if (mShape->IsInside(b)) {
						surfs_map->SelectItem<TsBiome, TsBiomeItem_SType>(
							b, surfs_items,
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
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: HeightMap exporting..."));
				int		reso  = mMapOutParam.LocalReso();
				FBox2D	bound = mMapOutParam.LocalBound(mBoundingbox);

				mHeightMap   = new TsHeightMap  (reso, reso, &bound);
				mMaterialMap = new TsMaterialMap(reso, reso, &bound);

				{///---------------------------------------- HeightMap
					mHeightMap->ForeachPixel(
						[&](int px, int py) {
							FVector2D p = mHeightMap->GetWorldPos(px, py);
							if (mHeightMap->IsWorld(p)) {
								TsBiome* b = SearchBiome(p);
								if (b) {
									mSurfaces[EBiomeSType::E_SurfNone].RemapHeight(b, p);
									mSurfaces[b->GetSType()          ].RemapHeight(b, p);
								}
								mShape->UpdateRemap(p);
							}
						});
					mHeightMap->ForeachPixel(
						[&](int px, int py) {
							FVector2D p = mHeightMap->GetWorldPos(px, py);
							TsBiome*  b = SearchBiome(p);
							if ( b ) {
								float h = 0;
								h += mSurfaces[EBiomeSType::E_SurfNone].GetHeight(b, p);
								h += mSurfaces[b->GetSType()          ].GetHeight(b, p) * b->GetMask(p);
								mHeightMap->SetPixel(px, py,h );
							}
						});
					UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: HeightMap done."));

					int out_reso = mMapOutParam.reso;
					int out_x    = mMapOutParam.LocalX();
					int out_y    = mMapOutParam.LocalY();
					mHeightMap->SaveAll(out_x, out_y, out_reso, out_reso);
				}
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
		mVoronoiSize, mVoronoiJitter,
		mReso,
		mErodeCycle
	);

	work->Debug( GetWorld());

#endif	//WITH_EDITOR

}

