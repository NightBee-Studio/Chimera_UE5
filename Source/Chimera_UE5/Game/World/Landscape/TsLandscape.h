#pragma once

#include "CoreMinimal.h"

//#include "Biome.h"
//#include "BiomeMap.h"
//#include "BiomeSurface.h"

#include "TsLandscape.generated.h"



// -------------------------------- UTsLandscape  --------------------------------
UCLASS(Blueprintable)
class CHIMERA_UE5_API UTsLandscape
	: public UObject
{
	GENERATED_BODY()

#if 0
public:
	TsBiomeSurface *				mBaseSurface;
	TMap<BiomeSrfType,BiomeSurface>	mSurfaces;

	TArray<Biome>					mBiomes;

	void*							mShape;
	FBox2D							mBoundingbox;
	float							mWaterLevel ;

	OutputConfig					mOutConfig;

	HeightMap *						mHeightMap;
	HeightMap *						mNormalMap;
	MaterialMap *					mMaterialMap;

private:
	float			GetHeight(const FVector2D& p);			// world-coord
	MaterialValue	GetMaterial(const FVector2D& p);		// world-coord

	void			UpdateRemap(const FVector2D& p);		// world-coord

	void			GetVoronoiList(BiomeGroup& list, TArray<Voronoi::Edge*>& edges, Biome* b, BiomeSrfType typ, int lvl);
	float			GetMaskValue(const FVector2D& p, BiomeSrfType biome_sf);

public:
	UTsLandscape();

	TsBiome*		SearchBiome(const FVector2D& p);		// world-coord

	void			Generate(float _x, float _y, float radius,
							 float	voronoi_size,
							 float	voronoi_jitter,
							 int	heightmap_reso,
							 int	erode_cycle );

	void			SetOutputConfig(const OutputConfig& conf) { mOutConfig = conf; }

	const FBox2D&	GetBoundingbox() const	{ return mBoundingbox ; }
	TsHeightMap*	GetHeightMap() const	{ return mHeightMap   ; }
	TsMaterialMap*	GetMaterialMap() const	{ return mMaterialMap ; }

	void			Release();

	void			Debug(UWorld* world);
#endif
};

