#define DECLARE

#include "SurfNoise.h"


float	SurfaceNoise::Remap(float val) const 
{
//	return mHeight * FMath::Pow(NoiseMap::Remap(val), mPowerFactor);
	return mHeight * TsNoiseMap::Remap(val) ;
}

float	SurfaceNoise::GetValue(const FVector2D& p) {
	return	( 1.00f * FMath::PerlinNoise2D(0.001f * mN0 * p + mNoisePos)
			+ 0.50f * FMath::PerlinNoise2D(0.002f * mN0 * p + mNoisePos)
			+ 0.25f * FMath::PerlinNoise2D(0.004f * mN0 * p + mNoisePos)
			+ 0.13f * FMath::PerlinNoise2D(0.008f * mN0 * p + mNoisePos)
			+ 0.06f * FMath::PerlinNoise2D(0.016f * mN0 * p + mNoisePos)
			+ 0.03f * FMath::PerlinNoise2D(0.032f * mN0 * p + mNoisePos));
}


float	SurfacePondNoise::Remap(float val) const
{
	return mHeight *
		//FMath::Clamp(
		//	TsNoiseMap::Remap(val),
		//	mThreshold-0.01f,
		//	mThreshold+0.01f) ;
		(TsNoiseMap::Remap(val) > mThreshold ? 1.0f : 0.0f);
}
