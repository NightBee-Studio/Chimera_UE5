#pragma once

#include "TsImageMap.h"

// ---------------------------- Utillity -------------------------
class TsMaskMap
	: //public TsValueMap
//	, 
public TsNoiseMap 
//	, public TsLevelMap
{
public:
	int					mN ;
float mGap ;
	TArray<FVector2D>	mPoints;

	TsMaskMap(float s, int n) ;

	virtual float	GetValue(const FVector2D& p) override;	// world-coord
} ;

