
#include "TsLandBuilder.h"

#include "Land/TsArea.h"
#include "Land/TsAreaMap.h"
#include "Land/TsLandShape.h"
#include "Land/TsLandSurface.h"

#include "Util/TsErosion.h"
#include "Util/TsVoronoi.h"


// ---------------------------- Utillity -------------------------

// nearest point 
static FVector2D near_point(const FVector2D& v1, const FVector2D& v2, const FVector2D& pt)
{
	FVector2D ap = pt - v1;
	FVector2D ab = v2 - v1;
	if (ap.Dot(ab) < 0) return v1;
	FVector2D bp = pt - v2;
	FVector2D ba = v1 - v2;
	if (bp.Dot(ba) < 0) return v2;
	return v1 + ab / ab.Length() * (ap.Dot(ab) / ab.Length());
}

// ID 
static int gen_pos_id(float x, float y, float dx = 0, float dy = 0) {
	unsigned int code = 0x0fffffff & (unsigned int)(x + 100 * y + dx + dy * 20);
	return (int)code + 1;
}

static float sigmoid(float x) {
	return tanh(x / 2);
	//	return (tanh(x/2) + 1)/2 - 0.5f ;
}

static inline FVector2D sincos_pos(float ang) {
	float x, y;
	FMath::SinCos(&y, &x, FMath::DegreesToRadians(ang));
	return FVector2D(x, y);
}



// ---------------------------- Builder -------------------------

class Builder_Work {
public:
	TMap<EBiomeSrfType, TsBiomeSurface>	mSurfaces;

	TArray<TsBiome>					mBiomes;

	TsLandShape*					mShape;

	FBox2D							mBoundingbox;
	float							mWaterLevel;

	TsMapOutput						mMapOutParam;

	TsHeightMap*					mHeightMap;
	TsHeightMap*					mNormalMap;
	TsMaterialMap*					mMaterialMap;

private:
	float			GetHeight(const FVector2D& p) {
		float h = mSurfaces[EBiomeSrfType::SurfBase].GetHeight(p);

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
		TsMaterialValue mv = mSurfaces[EBiomeSrfType::SurfBase].GetMaterial(p);
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
		mSurfaces[EBiomeSrfType::SurfBase].UpdateRemap(p);

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

	void			GetVoronoiList(TsBiomeGroup& list, TArray<TsVoronoi::Edge*>& edges, TsBiome* b, EBiomeSrfType typ, int lvl)
	{
		list.AddUnique(b);
		for (auto& ed : b->mEdges) {
			if (ed.mShared) {
				TsBiome* bm = (TsBiome*)ed.mShared;
				if (bm->GetBiomeSrfType() == typ) edges.Add(&ed);
				if (lvl > 0 && !list.Contains(bm)) {
					GetVoronoiList(list, edges, bm, typ, lvl - 1);
				}
			}
		}
	}

	float			GetMaskValue(const FVector2D& p, EBiomeSrfType biome_sf)
	{
		TsBiome* b = SearchBiome(p);
		float h = 0;
		if (b) {
			if (b->GetBiomeSrfType() == biome_sf) {
				h = 1;
			} else {
				TsBiomeGroup				bmlist;
				TArray<TsVoronoi::Edge*>	edlist;
				GetVoronoiList(bmlist, edlist, b, biome_sf, 2);
				for (auto ed : edlist) {
					float hc = 1 - (near_point(ed->mP, ed->mP + ed->mD, p) - p).Length() / 100;
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

	void			Debug(UWorld* world)
	{
//		SurfaceMountain::Debug(world);

		mShape->Debug(world);

		for (auto b : mBiomes) {
			FColor c(0, 0, 0);
			switch (b.GetBiomeSrfType()) {
			case EBiomeSrfType::SurfOcean:		c = FColor(0, 0, 180); break;
			case EBiomeSrfType::SurfLake:		c = FColor(100, 120, 255); break;
			case EBiomeSrfType::SurfField:		c = FColor(125, 255, 0); break;
			case EBiomeSrfType::SurfMountain:	c = FColor(120, 60, 20); break;
			}
			b.Debug(world, c);
		}
	}


public:
	Builder_Work()
		: mShape( new TsLandShape() )
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
			TArray<TsVoronoi::Edge >	boundedges;
			TArray<TsVoronoi::Point>	boundpoints;

			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Create Voronoi"));

			{//-------------------------------------------- boundary
				mShape->UpdateBoundingbox(mBoundingbox);
				mBoundingbox.Min -= FVector2D(500, 500);
				mBoundingbox.Max += FVector2D(500, 500);

				FVector2D		
					bp00(mBoundingbox.Min.X, mBoundingbox.Min.Y), bp10(mBoundingbox.Max.X, mBoundingbox.Min.Y),
					bp01(mBoundingbox.Min.X, mBoundingbox.Max.Y), bp11(mBoundingbox.Max.X, mBoundingbox.Max.Y);
				TsVoronoi::Edge
					be0(bp00, bp01, TsVoronoi::EFlag::IsBound), be1(bp01, bp11, TsVoronoi::EFlag::IsBound),
					be2(bp11, bp10, TsVoronoi::EFlag::IsBound), be3(bp10, bp00, TsVoronoi::EFlag::IsBound);

				boundedges.Add(be0);
				boundedges.Add(be1);
				boundedges.Add(be2);
				boundedges.Add(be3);
				boundpoints.Add(TsVoronoi::Point(bp00, be0.mID, be3.mID, TsVoronoi::EFlag::IsBound));
				boundpoints.Add(TsVoronoi::Point(bp01, be0.mID, be1.mID, TsVoronoi::EFlag::IsBound));
				boundpoints.Add(TsVoronoi::Point(bp11, be1.mID, be2.mID, TsVoronoi::EFlag::IsBound));
				boundpoints.Add(TsVoronoi::Point(bp10, be2.mID, be3.mID, TsVoronoi::EFlag::IsBound));
			}

			UE_LOG(LogTemp, Log, TEXT("UTsLandscape::    generate Voronoi-sites"));

			{//-------------------------------------------- generate random site.
				for (float x = mBoundingbox.Min.X; x < mBoundingbox.Max.X; x += voronoi_size) {
					for (float y = mBoundingbox.Min.Y; y < mBoundingbox.Max.Y; y += voronoi_size) {
						float px = x + FMath::RandRange(-voronoi_jitter, voronoi_jitter) * voronoi_size;
						float py = y + FMath::RandRange(-voronoi_jitter, voronoi_jitter) * voronoi_size;

						if (mBoundingbox.IsInside(FVector2D(px, py))) {
							mBiomes.Add(TsBiome(px, py));
						}
					}
				}
			}

			UE_LOG(LogTemp, Log, TEXT("UTsLandscape::    tesselate all  Voronoi-sites"));

			//-------------------------------------------- tesselate all sites
			for (auto& v : mBiomes) {
				TArray<TsBiome> voronois = mBiomes;
				// sort by the distance of the each voronoi-sites.
				voronois.Sort([v](const TsVoronoi& a, const TsVoronoi& b) { return (a - v).Length() < (b - v).Length(); });

				TArray<TsVoronoi::Edge>		edges;
				TArray<TsVoronoi::Point>	points;

				// add boundary edges
				edges.Append(boundedges);
				points.Append(boundpoints);

				int vertnum = 0;
				// under go with all voronoi-sites.
				for (auto& c : voronois) {
					if (c == v) continue;

					TsVoronoi::Edge edge((c.X + v.X) / 2, (c.Y + v.Y) / 2, (v.Y - c.Y), (c.X - v.X), TsVoronoi::EFlag::IsVoronoi);
					edges.Add(edge);

					if (edges.Num() > 1) {
						for (auto& e : edges) {						// add cross-point of new edges
							FVector2D pos;
							if (edge.Intersect(e, pos)) {
								points.AddUnique(TsVoronoi::Point(pos, e.mID, edge.mID, edge.mFlag | e.mFlag));
							}
						}
						for (auto& p : points) {						// check the point is inside of edge
							if (p.mID_a < 0 || p.mID_b < 0) continue;	// skip invalid edges
							for (auto& e : edges) {
								if (e.mID == p.mID_a || e.mID == p.mID_b) continue;

								auto cross = [&](const FVector2D& pn, const TsVoronoi::Edge& ed) { // cross product
									return ed.mD.X * (pn.Y - ed.mP.Y) - ed.mD.Y * (pn.X - ed.mP.X);
									};
								float d0 = cross(p, e);
								float d1 = cross(v, e);
								if (cross(p, e) * cross(v, e) < 0) {	// it is different side of the edge.
									p.mID_a = p.mID_b = -1;				// invalidate the point
									break;
								}
							}
						}
						for (auto& e : edges) {						// count reference of the edge
							e.mFlag &= ~TsVoronoi::EFlag::HasReference;
							for (auto p : points) {
								if (p.mID_a == e.mID) e.mFlag |= TsVoronoi::EFlag::HasReference;
								if (p.mID_b == e.mID) e.mFlag |= TsVoronoi::EFlag::HasReference;
							}
						}

						// remove all invalid points and edges.
						points.RemoveAll([](const TsVoronoi::Point& a) { return a.mID_a < 0 || a.mID_b < 0; });
						edges.RemoveAll([](const TsVoronoi::Edge& a) { return !(a.mFlag & TsVoronoi::EFlag::HasReference); });

						if (++vertnum > 10) break;///////////// this is maybe wrong.... what would be 
					}
				}

				{// refine the edge
					for (auto& e : edges) {
						TsVoronoi::Point pp[2];
						int        pi = 0;
						for (auto& p : points) {
							if (p.mID_a == e.mID || p.mID_b == e.mID) pp[pi++] = p;
							if (pi == 2) break;
						}
						if (pi == 1) {
							pp[1] = TsVoronoi::Point(pp[0] + FVector2D(e.mD.X, e.mD.Y) * 2, e.mID, 0, e.mFlag);
						}

						TsVoronoi::Edge	e01(pp[0], pp[1], e.mFlag);
						TsVoronoi::Edge	e10(pp[1], pp[0], e.mFlag);
						v.AddEdge(TsVoronoi::Edge(e01.Cross(v) < 0.0f ? e10 : e01));
					}
				}

				{// adjacent voronoi
					for (auto& ea : v.mEdges) {
						for (auto& vb : mBiomes) {
							if (v == vb) continue;
							bool done = false;
							for (auto& eb : vb.mEdges) {
								if (ea.mID == eb.mID) {
									ea.mShared = &vb;
									eb.mShared = &v;
									done = true;
									break;
								}
							}
							if (done) break;
						}
					}
				}
			}

			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Voronoi done."));

			//-------------------------------------------------------------------------------------- create voronois, done.
		}


		{///-------------------------------------------------------------------------------- Biome Group 		// you can access by 2d
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Biome Group"));

			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: BiomeMap creating ..."));

			if (heightmap_reso == 0) heightmap_reso = 512;
#define IMG_SIZE heightmap_reso

			mMapOutParam = TsMapOutput(0, 0, 512, 1);

			mSurfaces = TMap<EBiomeSrfType, TsBiomeSurface>{
				//{ BiomeSrfType::SurfField, }
				//{ BiomeSrfType::SurfOcean, TsBiomeSurface(512, mBoundingbox, "Demo/Landscape/Island/Masks/MaskOcean.dds", -1, { new SurfaceOcean(-5.0f) }, {}) },
				{ EBiomeSrfType::SurfBase, TsBiomeSurface(
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
				{ EBiomeSrfType::SurfLake, TsBiomeSurface(
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
				{ EBiomeSrfType::SurfMountain, TsBiomeSurface(
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

			TMap<EBiomeSrfType, TsBiomeGroup>	surf_biomes = {
				{ EBiomeSrfType::SurfOcean   , TsBiomeGroup{} },
				{ EBiomeSrfType::SurfLake    , TsBiomeGroup{} },
				{ EBiomeSrfType::SurfField   , TsBiomeGroup{} },
				{ EBiomeSrfType::SurfMountain, TsBiomeGroup{} },
			};

			// world once
			TsBiomeMap* surfs_map = new TsBiomeMap(IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.0010f, 0.2f, 0.0030f));
			// generate BiomeSrfType for determine the shape of biome group
			TArray<float>	samples;
			for (auto& b : mBiomes) {
				samples.Add(surfs_map->GetValue(b));
			}
			samples.Sort();		// sort to determine the ratio of the group.
			for (auto& b : mBiomes) {
				EBiomeSrfType surf = EBiomeSrfType::SurfOcean;		//, { 0.1f, 0.55f, 0.35f,}
				if (mShape->GetValue(b) > 0.99999f) {
					float h = surfs_map->GetValue(b);
					if      (h < samples[(samples.Num() - 1) * 0.05f])  surf = EBiomeSrfType::SurfLake;
					else if (h < samples[(samples.Num() - 1) * 0.50f])  surf = EBiomeSrfType::SurfField;
					else 												surf = EBiomeSrfType::SurfMountain;
				}

				b.SetBiomeSrfType(surf);
				surf_biomes[surf].Add(&b);
			}

			TsBiomeMap* moist_map  = new TsBiomeMap (IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.001f, 0.2f, 0.003f));
			TsGenreMap* genre_map  = new TsGenreMap (IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.001f, 0.2f, 0.003f));
			TsHeightMap* tempr_map = new TsHeightMap(IMG_SIZE, IMG_SIZE, &mBoundingbox);
			TsBiomeMap* slope_map  = new TsBiomeMap (IMG_SIZE, IMG_SIZE, &mBoundingbox);
			TsBiomeMap* plant_map  = new TsBiomeMap (IMG_SIZE, IMG_SIZE, &mBoundingbox, TsNoiseParam(1.0f, 0.002f, -0.2f, 0.010f));
			TsBiomeMap::AddBiomeMap(EBiomeMapType::BiomeMapMoist, moist_map);	//moisture
			TsBiomeMap::AddBiomeMap(EBiomeMapType::BiomeMapTempr, tempr_map);	//temperture
			TsBiomeMap::AddBiomeMap(EBiomeMapType::BiomeMapGenre, genre_map);	//genre
			TsBiomeMap::AddBiomeMap(EBiomeMapType::BiomeMapSlope, slope_map);	//sediment
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
			}
		}

	}

};




// -------------------------------- UTsLandscape  --------------------------------

UTsLandBuilder::UTsLandBuilder()
{
	mImplement = new Builder_Work();
}

void	UTsLandBuilder::Build(
	float _x, float _y, float radius,
	float	voronoi_size,
	float	voronoi_jitter,
	int		heightmap_reso,
	int		erode_cycle)
{
	Builder_Work * work = (Builder_Work*)mImplement ;

	work->BuildLandscape(
		_x, _y, radius,
		voronoi_size,
		voronoi_jitter,
		heightmap_reso,
		erode_cycle	
	);
}

