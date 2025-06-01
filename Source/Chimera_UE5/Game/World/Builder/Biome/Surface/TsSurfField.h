#pragma once

#include "../TsBiomeSurface.h"


class TsSurfaceField
	: public TsBiomeSFunc {
public:
	TsSurfaceField(const TsNoiseParam& cnf, float h = 0.2f)
		: TsBiomeSFunc(cnf, h) {}

	float	GetValue(const FVector2D& p) override;

//	float	GetHeight(TsBiome* b, const FVector2D& p) override;
};

