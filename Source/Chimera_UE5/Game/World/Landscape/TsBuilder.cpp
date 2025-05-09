
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
	float			GetHeight(const FVector2D& p) {
		float h = mSurfaces[EBiomeSType::E_SurfBase].GetHeight(p);

		if (mShape->GetValue(p) > 0.9999999f) {
			struct Tuple {
				float h, r;//	Tuple(float _h, float _r) :h(_h), r(_r) {}// height, ratio
			};
			TArray<Tuple> tuples;
			float total = 0;
			for (auto& sf : mSurfaces) {
				if (sf.Value.IsBase()) continue;
				float r = sf.Value.mMask->GetPixel(p);
				if (r > 0) {
					tuples.Add(Tuple{ sf.Value.GetHeight(p), r });
					total += r;
				}
			}
			if (total > 0) {
				total = FMath::Max(total, 1.0f);
				for (auto& tp : tuples) {
					h += tp.h * (tp.r / total);
				}
			}
		} else {
			h += mShape->Remap(mShape->GetValue(p));
		}
		return h;
	}


	TsMaterialValue	GetMaterial(const FVector2D& p)		// world-coord
	{
		TsMaterialValue mv = mSurfaces[EBiomeSType::E_SurfBase].GetMaterial(p);
		float r = mShape->GetMaterialValue(p);
		if (r == 0) {
			for (auto& sf : mSurfaces) {
				if (sf.Value.IsBase()) continue;
				r = sf.Value.mMask->GetPixel(p);
				if (r > 0) {
					mv.Merge(sf.Value.GetMaterial(p), FMath::Min(r, 1));
				}
			}
		} else {
			mv.Add(TsMaterialPixel(EMaterialType::MT_OceanSoil_A, r));
		}
		return  mv;
	}

	void			UpdateRemap(const FVector2D& p)		// world-coord
	{
		mSurfaces[EBiomeSType::E_SurfBase].UpdateRemap(p);

		if (mShape->GetValue(p) > 0.9999999f) {
			for (auto& sf : mSurfaces) {
				if (sf.Value.IsBase()) continue;

				float r = sf.Value.mMask->GetPixel(p);
				if (r > 0) {
					sf.Value.UpdateRemap(p);
				}
			}
		}
		else {
			mShape->UpdateRemap(p);
		}
	}

	void			GetVoronoiList(TsBiomeGroup& list, TArray<TsVoronoi::Edge*>& edges, TsBiome* b, EBiomeSType typ, int lvl)
	{
		list.AddUnique(b);
		for (auto& ed : b->mEdges) {
			if (ed.mShared) {
				TsBiome* bm = (TsBiome*)ed.mShared;
				if (bm->GetSType() == typ) edges.Add(&ed);
				if (lvl > 0 && !list.Contains(bm)) {
					GetVoronoiList(list, edges, bm, typ, lvl - 1);
				}
			}
		}
	}

	float			GetMaskValue(const FVector2D& p, EBiomeSType biome_sf)
	{
		TsBiome* b = SearchBiome(p);
		float h = 0;
		if (b) {
			if (b->GetSType() == biome_sf) {
				h = 1;
			} else {
				TsBiomeGroup				bmlist;
				TArray<TsVoronoi::Edge*>	edlist;
				GetVoronoiList(bmlist, edlist, b, biome_sf, 2);
				for (auto ed : edlist) {
					float hc = 1 - (TsUtil::NearPoint(ed->mP, ed->mP + ed->mD, p) - p).Length() / 100;
					hc = FMath::Pow(FMath::Clamp(hc, 0, 1), 2.0f);
					h = FMath::Max(hc, h);
				}
			}
		}
		return h;
	}


	TsBiome*		SearchBiome(const FVector2D& p)		// world-coord
	{
		for (auto& b : mBiomes) {
			if (b.IsInside(p)) return &b;
		}
		return nullptr;
	}

	void			SetOutputConfig(const TsMapOutput& conf) { mMapOutParam = conf; }

	const FBox2D&	GetBoundingbox() const { return mBoundingbox; }
	TsHeightMap*	GetHeightMap() const { return mHeightMap; }
	TsMaterialMap*	GetMaterialMap() const { return mMaterialMap; }

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
			case EBiomeSType::E_SurfOcean:		c = FColor(0, 0, 180); break;
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

			if (heightmap_reso == 0) heightmap_reso = 512;
#define IMG_SIZE heightmap_reso

			mMapOutParam = TsMapOutput(0, 0, 512, 1);
#if 0
			mSurfaces = TMap<EBiomeSType, TsBiomeSurface>{
				//{ BiomeSrfType::E_SurfField, }
				//{ BiomeSrfType::E_SurfOcean, TsBiomeSurface(512, mBoundingbox, "Demo/Landscape/Island/Masks/MaskOcean.dds", -1, { new SurfaceOcean(-5.0f) }, {}) },
				{ EBiomeSType::E_SurfBase, TsBiomeSurface(
						TsBiomeSurface::Flag::FL_BaseHeight,
						mMapOutParam.reso, mBoundingbox, "Landscape/Island/Masks/MaskField.dds",
						-1,
						{ new SurfaceField(TsNoiseParam(1.0f,0,0,0), 20.0f),
							new SurfacePondNoise(TsNoiseParam(0.8f, 0.010f, 0.2f, 0.030f), 0.5f, 0.4f),
						//new SurfaceField(TsNoiseParam(6.0f,0,0,0), 2.0f),
						},
						{ new TsBiomeMatFunc({
									{	EBiomeMapType::BiomeMapPlant,
										{	{ EMaterialType::MT_Soil_A  , 0.1f, nullptr, },//new Op_PixelGt(EBiomeMapType::BiomeMapTempr,0.3f)
											{ EMaterialType::MT_Grass_A , 0.3f,
											//new Op_ExecTask( new BiomeMatFunc( {
											//	{	EBiomeMapType::BiomeMapGenre,
											//		{	{ EMaterialType::MT_Moss_A, 0.4f, nullptr, },//new Op_PixelGt(EBiomeMapType::BiomeMapTempr,0.3f)
											//			{ EMaterialType::MT_Moss_B, 0.4f, nullptr, },
											//			{ EMaterialType::MT_Soil_B, 0.4f, nullptr, },
											//		}
											//	}
											//} ) ), 
											nullptr
										},
										{ EMaterialType::MT_Forest_B, 0.4f, nullptr, },
										{ EMaterialType::MT_Soil_B  , 0.2f, nullptr, },
									}
								},
							//{	EBiomeMapType::BiomeMapPond,
							//	{	{ EMaterialType::MT_Sand_A , 1.10f, nullptr, },
							//	},
							//},
						})
						}
					) },
				{ EBiomeSType::E_SurfLake, TsBiomeSurface(
						TsBiomeSurface::Flag::FL_None,
						IMG_SIZE, mBoundingbox, "Landscape\\Island\\Masks\\MaskLake.dds",
						-1,
						{	new SurfaceLake(TsNoiseParam(1.0f, 0.0010f, 0.2f, 0.0030f), -10.0f),
						},
						{	new TsBiomeMatFunc({
								{	EBiomeMapType::BiomeMapPlant,
									{	{ EMaterialType::MT_LakeSoil_A , 1.0f, nullptr, },
									}
								}
							}),
						}
					) },
				{ EBiomeSType::E_SurfMountain, TsBiomeSurface(
						TsBiomeSurface::Flag::FL_None,
						IMG_SIZE, mBoundingbox, "Landscape\\Island\\Masks\\MaskMount.dds",
						18,
						{	new SurfaceMountain(TsNoiseParam(1.0f, 0.0010f, 0.2f, 0.0030f), 20),
							new SurfacePondNoise(TsNoiseParam(0.8f, 0.010f, 0.2f, 0.030f), 0.5f, 0.5f),
							new SurfaceNoise(TsNoiseParam(10.0f, 0, 0, 0), 2.0f, 1.4f),
						},
						{	new TsBiomeMatFunc({
								{	EBiomeMapType::BiomeMapMountain,
									{	{ EMaterialType::MT_Grass_A, 0.50f, nullptr, },//new Op_PixelLt(EBiomeMapType::BiomeMapFlow,0.3f)
										{ EMaterialType::MT_Grass_B, 0.35f, nullptr, },//new Op_PixelLt(EBiomeMapType::BiomeMapFlow,0.3f)
										{ EMaterialType::MT_Soil_B , 0.10f, nullptr, },
										{ EMaterialType::MT_Soil_A , 0.05f, nullptr, },
									},
								},
								{	EBiomeMapType::BiomeMapFlow,
									{	{ EMaterialType::MT_Moss_B , 1.10f, nullptr, },
									},
								},
							}),
						}
					) },
			};
#endif
			TMap<EBiomeSType, TsBiomeGroup>	surf_biomes = {
				{ EBiomeSType::E_SurfOcean   , TsBiomeGroup{} },
				{ EBiomeSType::E_SurfLake    , TsBiomeGroup{} },
				{ EBiomeSType::E_SurfField   , TsBiomeGroup{} },
				{ EBiomeSType::E_SurfMountain, TsBiomeGroup{} },
			};

			////// apply the biome from PerlinNoise
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Apply Surface ..."));

			// world once
			TsBiomeMap* surfs_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.0010f, 0.2f, 0.0030f));
			// generate BiomeSrfType for determine the shape of biome group
			TArray<float>	samples;
			for (auto& b : mBiomes) {
				samples.Add(surfs_map->GetValue(b));
			}
			samples.Sort();		// sort to determine the ratio of the group.
			for (auto& b : mBiomes) {
				EBiomeSType surf = EBiomeSType::E_SurfOcean;		//, { 0.1f, 0.55f, 0.35f,}
				if (mShape->GetValue(b) > 0.99999f) {
					float h = surfs_map->GetValue(b);
					if      (h < samples[(samples.Num() - 1) * 0.05f])  surf = EBiomeSType::E_SurfLake;
					else if (h < samples[(samples.Num() - 1) * 0.50f])  surf = EBiomeSType::E_SurfField;
					else 												surf = EBiomeSType::E_SurfMountain;
				}

				b.SetSType(surf);
				surf_biomes[surf].Add(&b);
			}

#if 0
			TsBiomeMap* moist_map  = new TsBiomeMap (IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.001f, 0.2f, 0.003f));
			TsGenreMap* genre_map  = new TsGenreMap (IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.001f, 0.2f, 0.003f));
			TsHeightMap* tempr_map = new TsHeightMap(IMG_SIZE, IMG_SIZE, &mBoundingbox);
			TsBiomeMap* slope_map  = new TsBiomeMap (IMG_SIZE, IMG_SIZE, &mBoundingbox);
			TsBiomeMap* plant_map  = new TsBiomeMap (IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.002f, -0.2f, 0.010f));
			TsBiomeMap::AddBiomeMap(EBiomeMapType::BiomeMapMoist, moist_map);	//moisture
			TsBiomeMap::AddBiomeMap(EBiomeMapType::BiomeMapTempr, tempr_map);	//temperture
			TsBiomeMap::AddBiomeMap(EBiomeMapType::BiomeMapGenre, genre_map);	//genre
			//TsBiomeMap::AddBiomeMap(EBiomeMapType::BiomeMapSlope, slope_map);	//sediment
			TsBiomeMap::AddBiomeMap(EBiomeMapType::BiomeMapPlant, plant_map);	//plant

			for (auto& surf : mSurfaces) {// init surface 
				TsImageMap<float>* maskmap = surf.Value.CreateMask(IMG_SIZE, mBoundingbox);
				maskmap->ForeachPixel([&](int px, int py) {
					FVector2D	p = maskmap->GetWorldPos(px, py);
					maskmap->SetPixel(px, py, GetMaskValue(p, surf.Key));
					});
				maskmap->Save();// surf.Value.mFilename, ImageFile::Dds, ImageFormat::FormatL16);//"mask000.dds"

				for (auto bm : surf_biomes[surf.Key]) {
					surf.Value.GatherBiome(bm);
				}
				surf.Value.ForeachGroup();
			}

			{// create temperture-map
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape::    creating temperture-map..."));
				tempr_map->ForeachPixel([&](int px, int py) {
					FVector2D p = tempr_map->GetWorldPos(px, py);
					moist_map->UpdateRemap(p);
					genre_map->UpdateRemap(p);
					plant_map->UpdateRemap(p);
					UpdateRemap(p);
					});
				tempr_map->ForeachPixel([&](int px, int py) {
					FVector2D p = tempr_map->GetWorldPos(px, py);
					float temp = GetHeight(p);
					tempr_map->SetPixel(px, py, temp);
					});
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape::    temperture-map done."));
			}


			{// region maps
				int		reso = mMapOutParam.LocalReso();
				FBox2D	bound = mMapOutParam.LocalBound(mBoundingbox);

				TsBiomeMap* flow_map  = new TsBiomeMap(reso, reso, &bound);
				TsBiomeMap* pond_map  = new TsBiomeMap(reso, reso, &bound);
				TsMountMap* mount_map = new TsMountMap(reso, reso, &bound);
				TsBiomeMap::AddBiomeMap(EBiomeMapType::BiomeMapFlow, flow_map);	//flow
				TsBiomeMap::AddBiomeMap(EBiomeMapType::BiomeMapPond, pond_map);	//pond
				TsBiomeMap::AddBiomeMap(EBiomeMapType::BiomeMapMountain, mount_map);	//mount_map

				mHeightMap   = new TsHeightMap(reso, reso, &bound);
				mMaterialMap = new TsMaterialMap(reso, reso, &bound);
				int sy = mMapOutParam.x * mMapOutParam.reso;
				int sx = mMapOutParam.y * mMapOutParam.reso;
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape::    start create region[%d,%d]..."), sx, sy);
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape::    creating slope-map..."));
				slope_map->ForeachPixel([&](int px, int py) {
					FVector2D p = slope_map->GetWorldPos(px, py);
					mount_map->UpdateRemap(p);
					});
				slope_map->ForeachPixel([&](int px, int py) {
					FVector2D p = slope_map->GetWorldPos(px, py);
					FVector   n = tempr_map->CalcNormal(px, py);
					slope_map->SetPixel(px, py, FMath::Pow(n.Y, 5));
					mount_map->SetPixel(px, py, mount_map->GetValue(p));
					});
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape::    slope-map done."));

				// create the mask for each BiomeSurf types
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape::    creating surface-mask ..."));
				for (auto& surf : mSurfaces) {
					TsImageMap<float>* maskmap = surf.Value.CreateMask(reso, bound);
					maskmap->ForeachPixel([&](int px, int py) {
						maskmap->SetPixel(px, py, GetMaskValue(maskmap->GetWorldPos(px, py), surf.Key));
						});
					maskmap->Save();// surf.Value.mFilename, ImageFile::Dds, ImageFormat::FormatL16);//"mask000.dds"
				}
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape::    surface-mask done."));

				//for (auto& b : mBiomes) {
				//	float moist = moistmap->Remap(moistmap->GetValue(b));
				//	float genre = genremap->GetValue(b);
				//	b.SetBiomeType(BiomeType::BiomeTropical);
				//}
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: BiomeMap done."));


				mWaterLevel = 0.0f;
				// maybe we have to create the maskmap for merge field & mountain.

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

				{///--------------------------------------------------- File Exporting 
					TsImageCore::SetDir("Landscape\\Island\\BiomeMaps\\", mMapOutParam.x, mMapOutParam.y);

					UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: file exporting ..."));
					moist_map->Save("MoistMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
					genre_map->Save("GenreMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
					tempr_map->Save("TemprMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
					plant_map->Save("PlantMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
					flow_map ->Save("FlowMap.dds" , EImageFile::Dds, EImageFormat::FormatL16);
					pond_map ->Save("PondMap.dds" , EImageFile::Dds, EImageFormat::FormatL16);
					slope_map->Save("SlopeMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
					mount_map->Save("MountMap.dds", EImageFile::Dds, EImageFormat::FormatL16);
					UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: file export done."));
				}

				int out_reso = mMapOutParam.reso;
				int out_x    = mMapOutParam.LocalX();
				int out_y    = mMapOutParam.LocalY();
				mHeightMap  ->SaveAll(out_x, out_y, out_reso, out_reso);
				mMaterialMap->SaveAll(out_x, out_y, out_reso, out_reso);

				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: FileOut %d %d reso%d "), out_x, out_y, out_reso);


				//AActor* actor = GetWorld()->SpawnActor<AActor>(...);
				//if ( actor ) actor->SetFolderPath("MyFolder/SubFolder");

			}
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

