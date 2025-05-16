#pragma once

#include "../TsBiomeSurface.h"

class TsSurfacePondNoise : public TsBiomeSrfFunc {
private:
	float	mThreshold;		//0.0f - 1.0f
public:
	TsSurfacePondNoise(const TsNoiseParam& cnf, float thresh = 0.5f, float h = 0.2f)
		: TsBiomeSrfFunc(cnf, h), mThreshold(thresh) {}
	float	Remap(float val) const override;
};

class TsSurfaceNoise : public TsBiomeSrfFunc {
private:
	float	mPowerFactor;
public:
	TsSurfaceNoise(const TsNoiseParam& cnf, float pw=2.0f, float h=0.2f)
		: TsBiomeSrfFunc(cnf, h), mPowerFactor(pw) {}

	float	GetValue(const FVector2D& p) override;
	float	Remap(float val) const override;
};
