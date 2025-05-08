#define DECLARE

#include "SurfField.h"


float	SurfaceField::GetValue(const FVector2D& p) {
	return	( 1.00f * FMath::PerlinNoise2D(0.001f * mN0 * p + mNoisePos)
			+ 0.50f * FMath::PerlinNoise2D(0.002f * mN0 * p + mNoisePos)
			+ 0.25f * FMath::PerlinNoise2D(0.004f * mN0 * p + mNoisePos)
			+ 0.13f * FMath::PerlinNoise2D(0.008f * mN0 * p + mNoisePos)
			+ 0.06f * FMath::PerlinNoise2D(0.016f * mN0 * p + mNoisePos)
			+ 0.03f * FMath::PerlinNoise2D(0.032f * mN0 * p + mNoisePos));
}
