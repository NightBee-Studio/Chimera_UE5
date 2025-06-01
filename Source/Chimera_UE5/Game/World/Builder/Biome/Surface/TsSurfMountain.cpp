#include "TsSurfMountain.h"
#include "TsSurfUtility.h"


// -------------------------------- SurfaceMountain --------------------------------

float	TsSurfaceMountain::GetHeight(TsBiome* b, const FVector2D& p)
{
	TArray<TsBiome*> done_list;
	return TsSurfUtil::RecurseGetHeight( b, p, done_list, 1.0f );
}

float	TsSurfaceMountain::Remap(float val) const
{
	return mHeight * FMath::Pow(TsNoiseMap::Remap(val), mPowerFactor);
}

