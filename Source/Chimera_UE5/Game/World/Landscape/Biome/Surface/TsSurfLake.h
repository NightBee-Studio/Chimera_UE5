#pragma once

#include "../TsBiomeSurface.h"


class TsSurfaceLake
	: public TsBiomeSFunc {
public:
	TsSurfaceLake(const TsNoiseParam& cnf, float h = 0.2f)
		: TsBiomeSFunc(cnf, h) {}

	float	Remap(float val) const override; // { return  -8; }/////////////////////////////debug
	float	GetHeight(TsBiome* b, const FVector2D& p) override;
};
