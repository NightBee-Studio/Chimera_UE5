#pragma once

#include "TsImageMap.h"

// ---------------------------- Utillity -------------------------
class TsMaskMap
	: public TsNoiseMap 
{
public:
	TArray<FVector2D>	mPoints;

	TsMaskMap(float s) ;

	virtual float	GetValue(const FVector2D& p) override;	// world-coord
} ;

