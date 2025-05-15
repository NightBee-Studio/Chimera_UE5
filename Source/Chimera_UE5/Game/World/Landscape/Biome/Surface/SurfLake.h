#pragma once

#include "../TsBiomeSurface.h"


class SurfaceLake : public TsBiomeSrfFunc {
public:
	SurfaceLake(const TsNoiseParam& cnf, float h = 0.2f)
		: TsBiomeSrfFunc(cnf, h) {}

	//float	GetValue(const FVector2D& p) override;
	float	Remap(float val) const override; // { return  -8; }/////////////////////////////debug

	float	GetHeight(TsBiome* b, const FVector2D& p) override;

	//void	Exec_EachGroup(TsBiomeGroup& grp) override;
};
