#pragma once

#include "TsMaskMap.h"
#include "TsUtility.h"

TsMaskMap::TsMaskMap(float w, float h, int n)
		: TsValueMap()
		, TsLevelMap(w, h)
		, mN(n)
{
	float sx = w/n ;
	float sy = h/n ;
	for ( int y=0 ; y<n ; y++ ){
		for ( int x=0 ; x<n ; x++ ){
			float dx = x*sx + TsUtil::RandRange(-sx*0.3f, sx*0.3f) ;
			float dy = y*sy + TsUtil::RandRange(-sy*0.3f, sy*0.3f) ;
			mPoints.Add( FVector2D( dx,dy ) ) ;
		}
	}
}


float	TsMaskMap::GetValue(const FVector2D& p)
{
	const float powfactor = 0.7f ;

	float lv = 0 ;
	for ( auto &pn : mPoints ){
		float d = (p - pn).Length() ;
		float v = FMath::Clamp( FMath::Pow( d, powfactor ) / 100, 0.0f, 1.0f);
		lv = FMath::Max(v, lv);
	}
	return lv ;
}

