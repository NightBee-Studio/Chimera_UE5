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

float TsTextureMap::GetPixel(int x, int y, int reso ) 
{
	int channel = 0;///////
	int	byte_per_pixel = GPixelFormats[mTex->GetPixelFormat()].BlockBytes;
	int w = mTex->GetSizeX();
	int h = mTex->GetSizeY();
	int px = FMath::Clamp( (reso>0 ? w*x/reso : x), 0, w-1 ) ;
	int py = FMath::Clamp( (reso>0 ? h*y/reso : y), 0, h-1 ) ;
	uint8* data = (uint8*)mData + (px + py * w) * byte_per_pixel ;
	//UE_LOG(LogTemp, Log, TEXT("   GetPixel fmt%d [%d,%d]"), mTex->GetPixelFormat(), mTex->GetSizeX(), mTex->GetSizeY()) ;

	float  r = 0.0f;
	switch (mTex->GetPixelFormat()) {
	case EPixelFormat::PF_G16:
	case EPixelFormat::PF_R16_UINT:
	case EPixelFormat::PF_R16_SINT:
		r = ((uint16*)data)[channel] / 65535.0f;
		break;
	case EPixelFormat::PF_B8G8R8A8:
	case EPixelFormat::PF_R8G8B8A8:
		r = ((uint8*)data)[channel] / 255.0f;
		break;
	}
	return r;
}
