#pragma once

#include "../TsLandSurface.h"

class SurfacePondNoise : public TsBiomeSrfFunc {
private:
	float	mThreshold;		//0.0f - 1.0f
public:
	SurfacePondNoise(const TsNoiseParam& cnf, float thresh = 0.5f, float h = 0.2f)
		: TsBiomeSrfFunc(cnf, h), mThreshold(thresh) {}
	float	Remap(float val) const override;
};

class SurfaceNoise : public TsBiomeSrfFunc {
private:
	float	mPowerFactor;
public:
	SurfaceNoise(const TsNoiseParam& cnf, float pw=2.0f, float h=0.2f)
		: TsBiomeSrfFunc(cnf, h), mPowerFactor(pw) {}

	float	GetValue(const FVector2D& p) override;
	float	Remap(float val) const override;
};
