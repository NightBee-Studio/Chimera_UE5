#pragma once

#include <functional>
#include "CoreMinimal.h"


UENUM(BlueprintType)
enum ETextureMap
{
	ETM_Height,
	ETM_Genre,
	ETM_Flow,
	ETM_Wear,
	ETM_Slope,
	ETM_Deposite,
	ETM_Curvature,
} ;


class TsTextureMap 
{
public:
	TsTextureMap(UTexture2D *tex )
		: mTex(tex)
		, mData(nullptr) {}

	TObjectPtr<UTexture2D>	mTex;
	const void *			mData;

	bool	Lock();
	void	UnLock() ;

	float	GetPixel(int x, int y, int reso = 0);
};


