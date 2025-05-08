#pragma once

#include "../TsBiomeSurface.h"


class SurfaceField : public TsBiomeSrfFunc {
public:
	SurfaceField(const TsNoiseParam& cnf, float h = 0.2f)
		: TsBiomeSrfFunc(cnf, h) {}

	float	GetValue(const FVector2D& p) override;
};

