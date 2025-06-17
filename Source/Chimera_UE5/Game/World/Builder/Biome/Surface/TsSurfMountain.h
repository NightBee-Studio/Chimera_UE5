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
};
