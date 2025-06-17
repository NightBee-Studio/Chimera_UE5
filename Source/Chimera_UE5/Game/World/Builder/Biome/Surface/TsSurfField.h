#pragma once

#include "../TsBiomeSurface.h"


class TsSurfaceField
	: public TsBiomeSFunc {
public:
	TsSurfaceField( float s, float h = 0.2f)
		: TsBiomeSFunc(
			TsNoiseParam({
				{ 1.00f , 0.001f * s },
				{ 0.50f , 0.002f * s },
				{ 0.25f , 0.004f * s },
				{ 0.13f , 0.008f * s },
				{ 0.06f , 0.016f * s },
				{ 0.03f , 0.032f * s },
			}), h) {}

	//float	GetValue(const FVector2D& p) override;
};

