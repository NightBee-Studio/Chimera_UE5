#pragma once

#include "../TsBiomeSurface.h"


class TsSurfaceMountain
	: public TsBiomeSFunc {
private:
	float				mPowerFactor ;

public:
	TsSurfaceMountain( float h, float pw = 2.0f)
		: TsBiomeSFunc( TsNoiseParam(), h )
		, mPowerFactor(pw) {}

	float	Remap(float val) const override;
	float	GetHeight(TsBiome* b, const FVector2D& p) override;

public:
	static void *		gWork;
public:
	static void		Initialize( TArray<TsBiome>	& biome_list );
	static void		UpdateMountain( const FVector2D &pos, float h );
	static float	RemapMountain( const FVector2D &pos, float h );
};
