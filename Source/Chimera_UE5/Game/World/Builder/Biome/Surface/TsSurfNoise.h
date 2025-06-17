#pragma once

#include "../TsBiomeSurface.h"

class TsSurfacePondNoise
	: public TsBiomeSFunc {
private:
	float	mThreshold;		//0.0f - 1.0f
public:
	TsSurfacePondNoise(const TsNoiseParam& cnf, float thresh = 0.5f, float h = 0.2f)
		: TsBiomeSFunc(cnf, h)
		, mThreshold(thresh) {}
	float	Remap(float val) const override;
};

class TsSurfaceNoise
	: public TsBiomeSFunc {
private:
	float	mPowerFactor;
public:
	TsSurfaceNoise(float s, float pw=2.0f, float h=0.2f)
		: TsBiomeSFunc(
			TsNoiseParam({
				{1.00f,0.001f * s},
				{0.50f,0.002f * s},
				{0.25f,0.004f * s},
				{0.13f,0.008f * s},
				{0.06f,0.016f * s},
				{0.03f,0.032f * s},
				}), h), mPowerFactor(pw) {}

	//float	GetValue(const FVector2D& p) override;
	float	Remap(float val) const override;
};
