#pragma once


#include "TsVoronoi.h"
#include "../Biome/TsBiomeMap.h"


class TsHeightMap;
class TsBiomeMap;

/**
 * 
 */
class TsErosion
{
private:
	TsHeightMap*			mHeightMap   ;
	TsBiomeMap *			mFlowMap     ;
	TsBiomeMap *			mPondMap     ;

public:
	void				Simulate( int cycles); //Perform n erosion cycles

	static TArray<FVector2D> gDebug;

public:
	TsErosion(
		TsHeightMap*	heightmap,
		TsBiomeMap*		flowmap,
		TsBiomeMap*		pondmap
	) ;
	~TsErosion() {}
};
