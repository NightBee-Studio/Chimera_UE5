#pragma once

#include "../TsBiomeSurface.h"


class TsSurfaceLake
	: public TsBiomeSrfFunc {
public:
	TsSurfaceLake(const TsNoiseParam& cnf, float h = 0.2f)
		: TsBiomeSrfFunc(cnf, h) {}

	float	Remap(float val) const override; // { return  -8; }/////////////////////////////debug
	float	GetHeight(TsBiome* b, const FVector2D& p) override;
};
