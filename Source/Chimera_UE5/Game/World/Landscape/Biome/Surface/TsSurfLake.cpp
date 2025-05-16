#include "TsSurfLake.h"
#include "TsSurfUtility.h"


float	TsSurfaceLake::GetHeight(TsBiome* b, const FVector2D& p)
{
	TArray<TsBiome*> done_list;
	return TsSurfUtil::RecurseGetHeight(b, p, done_list, 50.0f );
}

float	TsSurfaceLake::Remap(float val) const
{
	return mHeight * FMath::Pow(TsNoiseMap::Remap(val), 0.5f );
}

