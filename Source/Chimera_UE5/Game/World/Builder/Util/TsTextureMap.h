#pragma once

#include <functional>
#include "CoreMinimal.h"


enum EAnchor{
	_Center	= 0x0000,
	_Right	= 0x0001,
	_Left	= 0x0002,
	_Upper	= 0x0004,
	_Bottom	= 0x0008,

	E_RU	= _Right |_Upper ,
	E_LU	= _Left  |_Upper ,
	E_CU	= _Center|_Upper ,
	E_RC	= _Right |_Center,
	E_LC	= _Left  |_Center,
	E_CC	= _Center|_Center,
	E_RB	= _Right |_Bottom,
	E_LB	= _Left  |_Bottom,
	E_CB	= _Center|_Bottom,
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

	float	GetPixel(int   x, int   y, EAnchor anc=E_LB, int reso = 0);
	float	GetValue(float x, float y, EAnchor anc=E_LB, int reso = 0);
} ;
