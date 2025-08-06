#pragma once

#include "TsMaskMap.h"
#include "TsUtility.h"

TsMaskMap::TsMaskMap(float s)
		: TsNoiseMap( { { 1.00f , 0.7f * s },
						{ 0.60f , 1.6f * s },
						{ 0.30f , 3.1f * s },
						{ 0.10f , 5.2f * s },} )
{
	const int n = 16 ;
	for ( int y=0 ; y<n ; y++ ){
		for ( int x=0 ; x<n ; x++ ){
			UpdateRemap( FVector2D(x,y) );
		}
	}
}


float	TsMaskMap::GetValue(const FVector2D& p)
{
	return TsNoiseMap::GetValue( p ) ;
}

