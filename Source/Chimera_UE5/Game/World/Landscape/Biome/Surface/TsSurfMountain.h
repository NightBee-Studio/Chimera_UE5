#pragma once

#include "../TsBiomeSurface.h"


class TsSurfaceMountain
	: public TsBiomeSFunc {
private:
	float				mPowerFactor ;

public:
	TsSurfaceMountain(const TsNoiseParam& cnf, float h, float pw = 2.0f)
		: TsBiomeSFunc(cnf, h), mPowerFactor(pw) {
		//gInstance = this;
	}

	float	Remap(float val) const override;
	float	GetHeight(TsBiome* b, const FVector2D& p) override;
};
