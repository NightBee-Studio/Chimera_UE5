#pragma once

#include <functional>
#include "CoreMinimal.h"


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


