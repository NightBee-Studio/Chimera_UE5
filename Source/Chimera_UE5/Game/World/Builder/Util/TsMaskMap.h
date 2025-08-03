#pragma once

#include "TsImageMap.h"

// ---------------------------- Utillity -------------------------
class TsMaskMap
	: public TsValueMap
	, public TsLevelMap {
public:
	int					mN ;
	TArray<FVector2D>	mPoints;

	TsMaskMap(float w, float h, int n) ;

	virtual float	GetValue(const FVector2D& p) override;	// world-coord
} ;

