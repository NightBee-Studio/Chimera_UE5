
#include "TsLandscape.h"


#if 0
#if WITH_EDITOR

#include "TsErosion.h"



static bool debugdddd=false;


bool dddddddddddddddd = false;



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


static
FVector2D near_point(const FVector2D& v1, const FVector2D& v2, const FVector2D& pp)
{
	FVector2D ap = pp - v1;
	FVector2D ab = v2 - v1;
	if (ap.Dot(ab) < 0) return v1;
	FVector2D bp = pp - v2;
	FVector2D ba = v1 - v2;
	if (bp.Dot(ba) < 0) return v2;
	return v1 + ab / ab.Length() * (ap.Dot(ab) / ab.Length());
}


static inline FVector2D sincos_pos(float ang) {
	float x, y;
	FMath::SinCos(&y, &x, FMath::DegreesToRadians(ang));
	return FVector2D(x, y);
}




// -------------------------------- UTsLandscape  --------------------------------

UTsLandscape::UTsLandscape()
	: mBaseSurface(nullptr )
	, mHeightMap(nullptr)
	, mNormalMap(nullptr)
	, mMaterialMap(nullptr)
{
	gInstance = this;
}

void	UTsLandscape::UpdateRemap(const FVector2D& p)
{
	mBaseSurface->UpdateRemap( p );

	if (mShape.GetValue(p) > 0.9999999f) {
		for (auto& sf : mSurfaces) {
			float r = sf.Value.mMask->GetPixel(p);
			if (r > 0) {
				sf.Value.UpdateRemap(p);
			}
		}
	} else {
		mShape.UpdateRemap(p);
	}
}

float	UTsLandscape::GetHeight(const FVector2D& p )
{
	float h = mBaseSurface->GetHeight(p);
	
	if (mShape.GetValue(p) > 0.9999999f) {
		struct Tuple {
			float h,r;//	Tuple(float _h, float _r) :h(_h), r(_r) {}// height, ratio
		};
		TArray<Tuple> tuples;
		float total = 0;
		for ( auto& sf : mSurfaces ) {
			float r = sf.Value.mMask->GetPixel(p);
			if (r > 0) {
				tuples.Add( Tuple{ sf.Value.GetHeight(p), r } );
				total += r;
			}
		}
		if (total > 0) {
			total = FMath::Max(total,1.0f) ;
			for (auto& tp : tuples) {
				h += tp.h * (tp.r / total);
			}
		}
	} else {
		h += mShape.Remap(mShape.GetValue(p));
	}
	return h;
}


MaterialValue	UTsLandscape::GetMaterial(const FVector2D& p)
{
	MaterialValue mv = mBaseSurface->GetMaterial(p);
	float r = mShape.GetMaterialValue(p) ;
	if ( r == 0 ) {
		for (auto& sf : mSurfaces) {
			r = sf.Value.mMask->GetPixel(p);
			if (r > 0) {
				mv.Merge( sf.Value.GetMaterial(p), FMath::Min(r, 1));
			}
		}
	} else {
		mv.Add( MaterialPixel(MaterialType::MT_OceanSoil_A, r) );
	}
	return  mv;
}




Biome*	UTsLandscape::SearchBiome(const FVector2D& p)
{
	for (auto& b : mBiomes) {
		if (b.IsInside(p)) return &b;
	}
	return nullptr;
}

void	UTsLandscape::GetVoronoiList(BiomeGroup& list, TArray<Voronoi::Edge*>& edges, Biome* b, BiomeSrfType typ, int lvl) {
	list.AddUnique(b);
	for (auto& ed : b->mEdges) {
		if (ed.mShared) {
			Biome* bm = (Biome*)ed.mShared;
			if (bm->GetBiomeSrfType() == typ) edges.Add(&ed);
			if (lvl > 0 && !list.Contains(bm)) {
				GetVoronoiList(list, edges, bm, typ, lvl - 1);
			}
		}
	}
}

float	UTsLandscape::GetMaskValue(const FVector2D& p, BiomeSrfType biome_sf)
{
	Biome* b = SearchBiome(p);
	float h = 0;
	if (b) {
		if (b->GetBiomeSrfType() == biome_sf) {
			h = 1;
		} else {
			BiomeGroup				bmlist;
			TArray<Voronoi::Edge*>	edlist;
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

#define SAFE_DELETE( var )	{if (var){ delete (var);(var)=nullptr;}}
void		UTsLandscape::Release()
{
	mBiomes.Empty();
	mShape.Release();

	SAFE_DELETE(mBaseSurface);
	SAFE_DELETE(mHeightMap);
	SAFE_DELETE(mNormalMap);
	SAFE_DELETE(mMaterialMap);
}


void	UTsLandscape::BuildLandscape(
			float	_x, float _y, float radius,
			float	voronoi_size,
			float	voronoi_jitter,
			int		heigthmap_reso,
			int		erode_cycle )
{
	//clean up first.
	Release();

	// create island shape
	mShape.Generate(_x, _y, radius);

	{//-------------------------------------------------------------------------------------- create voronois
		TArray<Voronoi::Edge >	boundedges;
		TArray<Voronoi::Point>	boundpoints;

		UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Create Voronoi") );

		{//-------------------------------------------- boundary
			mShape.UpdateBoundingbox(mBoundingbox);
			mBoundingbox.Min -= FVector2D(500,500);
			mBoundingbox.Max += FVector2D(500,500);

			FVector2D		bp00(mBoundingbox.Min.X, mBoundingbox.Min.Y), bp10(mBoundingbox.Max.X, mBoundingbox.Min.Y),
							bp01(mBoundingbox.Min.X, mBoundingbox.Max.Y), bp11(mBoundingbox.Max.X, mBoundingbox.Max.Y);
			Voronoi::Edge	be0 (bp00, bp01, Voronoi::Flag::IsBound), be1(bp01, bp11, Voronoi::Flag::IsBound),
							be2 (bp11, bp10, Voronoi::Flag::IsBound), be3(bp10, bp00, Voronoi::Flag::IsBound);

			boundedges.Add(be0);
			boundedges.Add(be1);
			boundedges.Add(be2);
			boundedges.Add(be3);
			boundpoints.Add(Voronoi::Point(bp00, be0.mID, be3.mID, Voronoi::Flag::IsBound));
			boundpoints.Add(Voronoi::Point(bp01, be0.mID, be1.mID, Voronoi::Flag::IsBound));
			boundpoints.Add(Voronoi::Point(bp11, be1.mID, be2.mID, Voronoi::Flag::IsBound));
			boundpoints.Add(Voronoi::Point(bp10, be2.mID, be3.mID, Voronoi::Flag::IsBound));
		}

		UE_LOG(LogTemp, Log, TEXT("UTsLandscape::    generate Voronoi-sites"));

		{//-------------------------------------------- generate random site.
			for (float x = mBoundingbox.Min.X; x < mBoundingbox.Max.X; x += voronoi_size) {
				for (float y = mBoundingbox.Min.Y; y < mBoundingbox.Max.Y; y += voronoi_size) {
					float px = x + FMath::RandRange(-voronoi_jitter, voronoi_jitter) * voronoi_size;
					float py = y + FMath::RandRange(-voronoi_jitter, voronoi_jitter) * voronoi_size;

					if (mBoundingbox.IsInside(FVector2D(px, py))) {
						mBiomes.Add(Biome(px, py));
					}
				}
			}
		}

		UE_LOG(LogTemp, Log, TEXT("UTsLandscape::    tesselate all  Voronoi-sites"));

		//-------------------------------------------- tesselate all sites
		for (auto& v : mBiomes) {
			TArray<Biome> voronois = mBiomes;
			// sort by the distance of the each voronoi-sites.
			voronois.Sort([v](const Voronoi& a, const Voronoi& b) { return (a - v).Length() < (b - v).Length(); });

			TArray<Voronoi::Edge>	edges;
			TArray<Voronoi::Point>	points;

			// add boundary edges
			edges .Append(boundedges );
			points.Append(boundpoints);

			int vertnum = 0;
			// under go with all voronoi-sites.
			for (auto& c : voronois) {
				if (c == v) continue;

				Voronoi::Edge edge((c.X + v.X) / 2, (c.Y + v.Y) / 2, (v.Y - c.Y), (c.X - v.X), Voronoi::Flag::IsVoronoi);
				edges.Add(edge);

				if (edges.Num() > 1) {
					for (auto& e : edges) {						// add cross-point of new edges
						FVector2D pos;
						if (edge.Intersect(e, pos)) {
							points.AddUnique(Voronoi::Point(pos, e.mID, edge.mID, edge.mFlag | e.mFlag));
						}
					}
					for (auto& p : points) {						// check the point is inside of edge
						if (p.mID_a < 0 || p.mID_b < 0) continue;	// skip invalid edges
						for (auto& e : edges) {
							if (e.mID == p.mID_a || e.mID == p.mID_b) continue;

							auto cross = [&](const FVector2D& pn, const Voronoi::Edge& ed) { // cross product
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
						e.mFlag &= ~Voronoi::Flag::HasReference;
						for (auto p : points) {
							if (p.mID_a == e.mID) e.mFlag |= Voronoi::Flag::HasReference;
							if (p.mID_b == e.mID) e.mFlag |= Voronoi::Flag::HasReference;
						}
					}

					// remove all invalid points and edges.
					points.RemoveAll([](const Voronoi::Point& a) { return a.mID_a < 0 || a.mID_b < 0; });
					edges .RemoveAll([](const Voronoi::Edge&  a) { return !(a.mFlag & Voronoi::Flag::HasReference) ; });

					if (++vertnum > 10) break;///////////// this is maybe wrong.... what would be 
				}
			}

			{// refine the edge
				for (auto& e : edges) {
					Voronoi::Point pp[2];
					int        pi = 0;
					for (auto& p : points) {
						if (p.mID_a == e.mID || p.mID_b == e.mID) pp[pi++] = p;
						if (pi == 2) break;
					}
					if (pi == 1) {
						pp[1] = Voronoi::Point(pp[0] + FVector2D(e.mD.X, e.mD.Y) * 2, e.mID, 0, e.mFlag);
					}

					Voronoi::Edge	e01(pp[0], pp[1], e.mFlag);
					Voronoi::Edge	e10(pp[1], pp[0], e.mFlag);
					v.AddEdge( Voronoi::Edge(e01.Cross(v) < 0.0f ? e10 : e01) );
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
								eb.mShared = &v ;
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

		if (heigthmap_reso == 0) heigthmap_reso = 512;
#define IMG_SIZE heigthmap_reso

		mOutConfig = OutputConfig(0,0, 512, 1);

		mBaseSurface =
			new BiomeSurface(
				BiomeSurface::Flag::FL_BaseHeight,
				mOutConfig.reso, mBoundingbox, "Landscape/Island/Masks/MaskField.dds",
				-1,
				{	new SurfaceField(NoiseConfig(1.0f,0,0,0), 20.0f),
					new SurfacePondNoise(NoiseConfig(0.8f, 0.010f, 0.2f, 0.030f), 0.5f, 0.4f),
					//new SurfaceField(NoiseConfig(6.0f,0,0,0), 2.0f),
				},
				{	new BiomeMatFunc( {
							{	BiomeMapType::BiomeMapPlant,
								{	{ MaterialType::MT_Soil_A  , 0.1f, nullptr, },//new Op_PixelGt(BiomeMapType::BiomeMapTempr,0.3f)
									{ MaterialType::MT_Grass_A , 0.3f,
										//new Op_ExecTask( new BiomeMatFunc( {
										//	{	BiomeMapType::BiomeMapGenre,
										//		{	{ MaterialType::MT_Moss_A, 0.4f, nullptr, },//new Op_PixelGt(BiomeMapType::BiomeMapTempr,0.3f)
										//			{ MaterialType::MT_Moss_B, 0.4f, nullptr, },
										//			{ MaterialType::MT_Soil_B, 0.4f, nullptr, },
										//		}
										//	}
										//} ) ), 
										nullptr
									},
									{ MaterialType::MT_Forest_B, 0.4f, nullptr, },
									{ MaterialType::MT_Soil_B  , 0.2f, nullptr, },
								}
							},
							//{	BiomeMapType::BiomeMapPond,
							//	{	{ MaterialType::MT_Sand_A , 1.10f, nullptr, },
							//	},
							//},
						} )
				}
		);

		mSurfaces = TMap<BiomeSrfType, BiomeSurface>{
				//{ BiomeSrfType::SurfField, }
				//{ BiomeSrfType::SurfOcean, BiomeSurface(512, mBoundingbox, "Demo/Landscape/Island/Masks/MaskOcean.dds", -1, { new SurfaceOcean(-5.0f) }, {}) },
				{ BiomeSrfType::SurfLake, BiomeSurface(
						BiomeSurface::Flag::FL_None,
						IMG_SIZE, mBoundingbox, "Landscape\\Island\\Masks\\MaskLake.dds",
						-1,
						{	new SurfaceLake( NoiseConfig(1.0f, 0.0010f, 0.2f, 0.0030f), -10.0f ),
						},
						{	new BiomeMatFunc({
								{	BiomeMapType::BiomeMapPlant,
									{	{ MaterialType::MT_LakeSoil_A , 1.0f, nullptr, },
									}
								}
							}),
						}
					) },
				{ BiomeSrfType::SurfMountain, BiomeSurface(
						BiomeSurface::Flag::FL_None,
						IMG_SIZE, mBoundingbox, "Landscape\\Island\\Masks\\MaskMount.dds",
						18,
						{	new SurfaceMountain( NoiseConfig(1.0f, 0.0010f, 0.2f, 0.0030f), 20 ),
							new SurfacePondNoise( NoiseConfig(0.8f, 0.010f, 0.2f, 0.030f), 0.5f, 0.5f),
							new SurfaceNoise( NoiseConfig(10.0f, 0, 0, 0), 2.0f, 1.4f),
						},
						{	new BiomeMatFunc({ 
								{	BiomeMapType::BiomeMapMountain,
									{	{ MaterialType::MT_Grass_A, 0.50f, nullptr, },//new Op_PixelLt(BiomeMapType::BiomeMapFlow,0.3f)
										{ MaterialType::MT_Grass_B, 0.35f, nullptr, },//new Op_PixelLt(BiomeMapType::BiomeMapFlow,0.3f)
										{ MaterialType::MT_Soil_B , 0.10f, nullptr, },
										{ MaterialType::MT_Soil_A , 0.05f, nullptr, },
									},
								},
								{	BiomeMapType::BiomeMapFlow,
									{	{ MaterialType::MT_Moss_B , 1.10f, nullptr, },
									},
								},
							}),
						}
					) },
		};

		TMap<BiomeSrfType, BiomeGroup>	surf_biomes = {
			{ BiomeSrfType::SurfOcean   , BiomeGroup{} },
			{ BiomeSrfType::SurfLake    , BiomeGroup{} },
			{ BiomeSrfType::SurfField   , BiomeGroup{} },
			{ BiomeSrfType::SurfMountain, BiomeGroup{} },
		};

		// world once
		BiomeMap*	surfs_map = new BiomeMap (IMG_SIZE, IMG_SIZE, &mBoundingbox, NoiseConfig(1.0f, 0.0010f, 0.2f, 0.0030f));
		// generate BiomeSrfType for determine the shape of biome group
		TArray<float>	samples;
		for (auto& b : mBiomes) {
			samples.Add(surfs_map->GetValue(b));
		}
		samples.Sort();		// sort to determine the ratio of the group.
		for (auto& b : mBiomes) {
			BiomeSrfType surf = BiomeSrfType::SurfOcean;		//, { 0.1f, 0.55f, 0.35f,}
			if (mShape.GetValue(b) > 0.99999f) {
				float h = surfs_map->GetValue(b);
				if      (h < samples[(samples.Num() - 1) * 0.05f])  surf = BiomeSrfType::SurfLake;
				else if (h < samples[(samples.Num() - 1) * 0.50f])  surf = BiomeSrfType::SurfField;
				else 												surf = BiomeSrfType::SurfMountain;
			}

			b.SetBiomeSrfType(surf);
			surf_biomes[surf].Add(&b);
		}

		BiomeMap*  moist_map = new BiomeMap (IMG_SIZE, IMG_SIZE, &mBoundingbox, NoiseConfig(1.0f, 0.001f, 0.2f, 0.003f));
		GenreMap*  genre_map = new GenreMap (IMG_SIZE, IMG_SIZE, &mBoundingbox, NoiseConfig(1.0f, 0.001f, 0.2f, 0.003f));
		HeightMap* tempr_map = new HeightMap(IMG_SIZE, IMG_SIZE, &mBoundingbox);
		BiomeMap*  slope_map = new BiomeMap (IMG_SIZE, IMG_SIZE, &mBoundingbox);
		BiomeMap*  plant_map = new BiomeMap (IMG_SIZE, IMG_SIZE, &mBoundingbox, NoiseConfig(1.0f, 0.002f,-0.2f, 0.010f));
		BiomeMap::AddBiomeMap(BiomeMapType::BiomeMapMoist, moist_map);	//moisture
		BiomeMap::AddBiomeMap(BiomeMapType::BiomeMapTempr, tempr_map);	//temperture
		BiomeMap::AddBiomeMap(BiomeMapType::BiomeMapGenre, genre_map);	//genre
		BiomeMap::AddBiomeMap(BiomeMapType::BiomeMapSlope, slope_map);	//sediment
		BiomeMap::AddBiomeMap(BiomeMapType::BiomeMapPlant, plant_map);	//plant

		for (auto& surf : mSurfaces) {// init surface 
			ImageMap<float>* maskmap = surf.Value.CreateMask( IMG_SIZE, mBoundingbox );
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
			int		reso  = mOutConfig.LocalReso() ;
			FBox2D	bound = mOutConfig.LocalBound(mBoundingbox);

			BiomeMap* flow_map  = new BiomeMap(reso, reso, &bound);
			BiomeMap* pond_map  = new BiomeMap(reso, reso, &bound);
			MountMap* mount_map = new MountMap(reso, reso, &bound);
			BiomeMap::AddBiomeMap(BiomeMapType::BiomeMapFlow    , flow_map );	//flow
			BiomeMap::AddBiomeMap(BiomeMapType::BiomeMapPond    , pond_map );	//pond
			BiomeMap::AddBiomeMap(BiomeMapType::BiomeMapMountain, mount_map);	//mount_map

			mHeightMap   = new HeightMap  (reso, reso, &bound);
			mMaterialMap = new MaterialMap(reso, reso, &bound);
			int sy = mOutConfig.x * mOutConfig.reso;
			int sx = mOutConfig.y * mOutConfig.reso;
			UE_LOG(LogTemp, Log, TEXT("UTsLandscape::    start create region[%d,%d]..."),sx,sy);
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
				ImageMap<float>* maskmap = surf.Value.CreateMask(reso, bound);
				maskmap->ForeachPixel([&](int px, int py) {
						maskmap->SetPixel( px, py, GetMaskValue( maskmap->GetWorldPos(px, py), surf.Key ));
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
				TArray<Voronoi> v_list;
				SurfaceMountain::GetMountains(v_list);
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: creating Erosion  mountinas %d ..."), v_list.Num());

				Erosion(mHeightMap, flow_map, pond_map).Simulate(erode_cycle);

				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Erosion done."));
			}

			{///--------------------------------------------------- Generate MaterialMap
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: MaterialMap preparing..."));

				// update Occupancy of the config...
				BiomeMatFunc::UpdateOccupancy();

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
				ImageCore::SetDir("Landscape\\Island\\BiomeMaps\\", mOutConfig.x, mOutConfig.y );

				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: file exporting ..."));
				moist_map->Save("MoistMap.dds", ImageFile::Dds, ImageFormat::FormatL16);
				genre_map->Save("GenreMap.dds", ImageFile::Dds, ImageFormat::FormatL16);
				tempr_map->Save("TemprMap.dds", ImageFile::Dds, ImageFormat::FormatL16);
				plant_map->Save("PlantMap.dds", ImageFile::Dds, ImageFormat::FormatL16);
				flow_map-> Save("FlowMap.dds" , ImageFile::Dds, ImageFormat::FormatL16);
				pond_map-> Save("PondMap.dds" , ImageFile::Dds, ImageFormat::FormatL16);
				slope_map->Save("SlopeMap.dds", ImageFile::Dds, ImageFormat::FormatL16);
				mount_map->Save("MountMap.dds", ImageFile::Dds, ImageFormat::FormatL16);
				UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: file export done."));
			}

			int out_reso = mOutConfig.reso;
			int out_x	= mOutConfig.LocalX();
			int out_y	= mOutConfig.LocalY();
			mHeightMap  ->SaveAll(out_x, out_y, out_reso, out_reso);
			mMaterialMap->SaveAll(out_x, out_y, out_reso, out_reso);

			UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: FileOut %d %d reso%d "), out_x, out_y, out_reso);
		}
	}

}



void	UTsLandscape::Debug(UWorld* world)
{
	SurfaceMountain::Debug( world );

	mShape.Debug(world);

	for (auto b : mBiomes) {
		FColor c(0, 0, 0);
		switch( b.GetBiomeSrfType() ){
		case BiomeSrfType::SurfOcean:		c = FColor(  0,  0,180); break;
		case BiomeSrfType::SurfLake:		c = FColor(100,120,255); break;
		case BiomeSrfType::SurfField:		c = FColor(125,255,  0); break;
		case BiomeSrfType::SurfMountain:	c = FColor(120, 60, 20); break;
		}
		b.Debug(world, c);
	}
}


UTsLandscape* UTsLandscape::gInstance = nullptr;


#endif	//WITH_EDITOR

#endif	

