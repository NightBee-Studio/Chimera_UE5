#include "TsTextureMap.h"

//#include "TsUtility.h"


bool TsTextureMap::Lock() 
{
	if (
#if WITH_EDITORONLY_DATA
		mTex->Source.IsValid()
#else
		mTex->GetPlatformData() && mTex->GetPlatformData()->Mips.Num() > 0
#endif
		) {
#if WITH_EDITORONLY_DATA
		//UE_LOG(LogTemp, Log, TEXT("Source  IsValid%d Fmt%d"), tex ? tex->Source.IsValid() : -2, tex->GetPixelFormat());
		mData = (const void*)mTex->Source.LockMip(0);
#else
		//UE_LOG(LogTemp, Log, TEXT("Platform MipNum%d"), tex ? tex->GetPlatformData() ? tex->GetPlatformData()->Mips.Num() : -2 : -1);
		mData = mTex->GetPlatformData()->Mips[0].BulkData.LockReadOnly();
#endif
		return true;
	}
	return false;
}

void TsTextureMap::UnLock()
{
	if ( mData ){
	#if WITH_EDITORONLY_DATA
		mTex->Source.UnlockMip(0);
	#else
		mTex->GetPlatformData()->Mips[0].BulkData.Unlock();
	#endif
	}
	mData = nullptr;
}


int	TsTextureMap::GetSizeX() const 
{
	return mTex->GetSizeX();
}

int	TsTextureMap::GetSizeY() const 
{
	return mTex->GetSizeY();
}

float TsTextureMap::GetPixel(int x, int y, EAnchor anc, int reso ) 
{
	int channel = 0;///////
	int w = GetSizeX();
	int h = GetSizeY();

int ox=x, oy =y;
	//anchor control
	switch( anc & (_Right|_Left  ) ){
	case EAnchor::_Left:	           break ;
	case EAnchor::_Center:	x = x+w/2; break ;
	case EAnchor::_Right:	x = w-  x; break ;
	}
	switch( anc & (_Upper|_Bottom) ){
	case EAnchor::_Bottom:	           break ;
	case EAnchor::_Center:	y = y+h/2; break ;
	case EAnchor::_Upper:	y = h-  y; break ;
	}

	int px = FMath::Clamp( (reso>0 ? w*x/reso : x), 0, w-1 ) ;
	int py = FMath::Clamp( (reso>0 ? h*y/reso : y), 0, h-1 ) ;
	int	byte_per_pixel = GPixelFormats[mTex->GetPixelFormat()].BlockBytes;
	uint8* data = (uint8*)mData + (px + py * w) * byte_per_pixel ;

//	UE_LOG(LogTemp, Log, TEXT("GetPixel o(%d %d)=>(%d %d)=>p(%d %d)fmt%d"),ox,oy,x,y,px,py, mTex->GetPixelFormat() );

	float  r = 0.0f;
	switch (mTex->GetPixelFormat()) {
	case EPixelFormat::PF_G16:
	case EPixelFormat::PF_R16_UINT:
	case EPixelFormat::PF_R16_SINT:
		r = ((uint16*)data)[channel] / 65535.0f;
		break;
	case EPixelFormat::PF_G8:
	case EPixelFormat::PF_B8G8R8A8:
	case EPixelFormat::PF_R8G8B8A8:
		r = ((uint8*)data)[channel] / 255.0f;
		break;
	default:
		UE_LOG(LogTemp, Log, TEXT("TsTextureMap : Unknown format%d"), mTex->GetPixelFormat() );
		break;
	}
	return r;
}

float	TsTextureMap::GetValue(float x, float y, EAnchor anc, int reso )
{
	int   dx  = (int)x ;
	int   dy  = (int)y ;
	float sx  = FMath::Frac(x) ;
	float sy  = FMath::Frac(y) ;
	float h00 = TsTextureMap::GetPixel( dx+0, dy+0, anc, reso ) ;
	float h01 = TsTextureMap::GetPixel( dx+1, dy+0, anc, reso ) ;
	float h10 = TsTextureMap::GetPixel( dx+0, dy+1, anc, reso ) ;
	float h11 = TsTextureMap::GetPixel( dx+1, dy+1, anc, reso ) ;
	return ((h00 * (1-sx) + h01*sx) * (1-sy)+
			(h10 * (1-sx) + h11*sx) * (  sy)) ;
}

