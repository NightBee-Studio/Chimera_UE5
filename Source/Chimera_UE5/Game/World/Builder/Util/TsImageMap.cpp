#include "TsImageMap.h"

#include <stdio.h>

#include "GenericPlatform/GenericPlatformFile.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Texture2D.h"
//#include "PackageTools.h"

#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"




// -------------------------------- TsMapOutput  --------------------------------

//FBox2D	TsMapOutput::WorldBound( int x, int y )
//{
//	return TsUtil::TsBox( mX+x*mReso, mY+y*mReso, mReso, mReso );
//
//	return FBox2D(	world_bound.Min + size * FVector2D( mX-0.5f, mY-0.5f ),
//					world_bound.Min + size * FVector2D( mX+1.5f, mY+1.5f ) );
//}

FBox2D	TsMapOutput::LocalBound( int x, int y, int reso )
{
	if (mDiv == 1) return *mWorldBound;

	float ratio = mWorldBound->GetSize().X / mWorldReso ;
	float sx    = mX   * ratio ;
	float sy    = mY   * ratio ;
	float size  = reso * ratio ;

	FVector2D	min = mWorldBound->Min + FVector2D( size*x, size*y ) + FVector2D( sx, sy );
	FVector2D	max = min              + FVector2D( size  , size   ) ;
	return FBox2D( min, max );
}

TsUtil::TsBox	TsMapOutput::TexBound( int x, int y, int reso )
{
	return TsUtil::TsBox( mX+x*reso, mY+y*reso, reso, reso );
}



// -------------------------------- TsValueMap  --------------------------------
void	TsValueMap::ResetRemap() 
{
	mMin =  10000.0f;
	mMax = -10000.0f;
}

float	TsValueMap::Remap(float val) const 
{
	return  (mMax - mMin) > 0 ? (val - mMin) / (mMax - mMin) : 1;
}

void	TsValueMap::RemapDone() 
{
	mMax = mMin = 0 ;
}

void	TsValueMap::UpdateRemap(const FVector2D& p) 
{
	float v = GetValue(p) ;
	mMin = FMath::Min(mMin, v);
	mMax = FMath::Max(mMax, v);
}


// -------------------------------- TsNoiseMap  --------------------------------
void TsNoiseParam::Setup(int seed )
{
	//if (seed != -1) TsUtil::RandSeed(seed);
	mNoisePos = FVector2D(TsUtil::RandRange(0, 1024), TsUtil::RandRange(0, 1024));
}

float	TsNoiseMap::GetValue(const FVector2D& p) {
	float val = 0;
	for ( const auto & l : mLayers) {
		val += l.X * FMath::PerlinNoise2D(l.Y * p + mNoisePos);
	}
	return val;
}

//	switch (mType) {
//	default:
//	case ENoiseType::NoiseNormal:
//		return (    mN0 * FMath::PerlinNoise2D(         mS0 * p + mNoisePos)
//				  + mN1 * FMath::PerlinNoise2D(         mS1 * p + mNoisePos));
//	case ENoiseType::NoiseMaterial:
//		return (  0.80f * FMath::PerlinNoise2D(0.001f * mN0 * p + mNoisePos)
//				+ 0.50f * FMath::PerlinNoise2D(0.002f * mN0 * p + mNoisePos)
//				+ 0.25f * FMath::PerlinNoise2D(0.004f * mN0 * p + mNoisePos)
//				+ 0.13f * FMath::PerlinNoise2D(0.008f * mN0 * p + mNoisePos)
//				+ 0.60f * FMath::PerlinNoise2D(0.016f * mN0 * p + mNoisePos)
//				+ 0.30f * FMath::PerlinNoise2D(0.032f * mN0 * p + mNoisePos));
//	case ENoiseType::NoiseField:
//		return (  1.00f * FMath::PerlinNoise2D(0.001f * mN0 * p + mNoisePos)
//				+ 0.50f * FMath::PerlinNoise2D(0.002f * mN0 * p + mNoisePos)
//				+ 0.25f * FMath::PerlinNoise2D(0.004f * mN0 * p + mNoisePos)
//				+ 0.13f * FMath::PerlinNoise2D(0.008f * mN0 * p + mNoisePos)
//				+ 0.06f * FMath::PerlinNoise2D(0.016f * mN0 * p + mNoisePos)
//				+ 0.03f * FMath::PerlinNoise2D(0.032f * mN0 * p + mNoisePos));
//	}
//}


// -------------------------------- TsImageCore  --------------------------------


// Format                                    -------------------------------- BMP 
//
struct	BITMAPFILEHEADER {	// BMP struct size will be 16, but Bmp-Format must be 14...
	char    bfType[2];
	int		bfSize;
	short	bfReserved1;
	short	bfReserved2;
	int		bfOffBits;
};

struct	BITMAPINFOHEADER {
	UINT32	biSize;//�w�b�_�T�C�Y	40 - Windows Bitmap	12 - OS/2 Bitmap
	UINT32	biWidth;//�摜�̕� (�s�N�Z��)
	UINT32	biHeight;//�摜�̍��� (�s�N�Z��)
	INT16	biPlanes;//�v���[�����@��� 1
	INT16	biBitCount;//1��f������̃f�[�^�T�C�Y(bit)	��j256 �F�r�b�g�}�b�v �� 8
	UINT32	biCompression;//���k�`��	0:BI_RGB(�����k)	1:BI_RLE8 (RunLength 8 bits/pixel)	2:BI_RLE4 (RunLength 4 bits/pixel)	3:BI_BITFIELDS (Bitfields)	(4 - BI_JPEG)	(5 - BI_PNG)
	UINT32	biSizeImage;//�摜�f�[�^���̃T�C�Y(byte)
	UINT32	biXPelsPerMeter;//�������𑜓x (1m������̉�f��)	//96dpi �Ȃ��3780
	UINT32	biYPelsPerMeter;//�c�����𑜓x (1m������̉�f��)	//96dpi �Ȃ��3780
	UINT32	biClrUsed;//�i�[����Ă���p���b�g�� (�g�p�F��)
	UINT32	biClrImportant;//�d�v�ȃp���b�g�̃C���f�b�N�X		//0 �̏ꍇ������
};


// Format                                    -------------------------------- PNG 
//
struct	PNGTYPE {	// BMP struct size will be 16, but Bmp-Format must be 14...
	UINT8    pnType[8];
};
struct	PNGHEADER {			// ChunkType =49 48 44 52	"IHDR" 
	UINT32	pnWidth;		//�摜�̕� (�s�N�Z��)
	UINT32	pnHeight;		//�摜�̍��� (�s�N�Z��)
	UINT8	pnDepth;			//	�r�b�g�[�x	(�L���Ȓl�� 1, 2, 4, 8, 16 �����A
	UINT8	pnColorType;	//	�J���[�^�C�v(�L���Ȓl�� 0, 2, 3, 4, 6)
	UINT8	pnCompressType;	//	���k��@
	UINT8	pnFilterType;	//	�t�B���^�[��@
	UINT8	pnInteraceType;	//�C���^�[���[�X��@
	UINT32	pnCRC;			// (Cyclic Redundancy Check)	Chunk Type �� Chunk Data �� ���ƂɌv�Z�����
};
enum    PNGColorType {
	HasAlpha   = 0x04,
	HasRGB     = 0x02,
	HasPalette = 0x01,

	GrayScale   = 0,
	RGBIndex    = HasRGB | HasPalette,
	RGBColor	= HasRGB,
	RGBAColor	= HasRGB | HasAlpha,
	GrayAlpha   = HasAlpha,
};
struct	PNGChunkBegin {
	UINT32	pnLength;
	UINT32	pnChunkType;
};
struct	PNGChunkEnd {
	UINT32	pnCRC;
};


// Format                                    -------------------------------- DDS
//
struct DDSHEADER {
	DWORD   dwMagic				;// == ��� 0x20534444  ' SDD'
	DWORD   dwSize				;// == ��� 124
	DWORD   dwFlags				;// �w�b�_���̗L���ȏ�� DDSD_* �̑g�ݍ��킹
	DWORD   dwHeight			;// �摜�̍��� x size
	DWORD   dwWidth				;// �摜�̕�   y size
	DWORD   dwPitchOrLinearSize	;// ��1 line �� byte �� (pitch) �܂��� 1�ʕ��� byte �� (linearsize)
	DWORD   dwDepth				;// �摜�̉��s�� z size (Volume Texture �p)
	DWORD   dwMipMapCount		;// �܂܂�Ă��� mipmap ���x����
	DWORD   dwReserved1[11];
	DWORD   dwPfSize			;// == ��� 32
	DWORD   dwPfFlags			;// pixel �t�H�[�}�b�g��\�� DDPF_* �̑g�ݍ��킹
	DWORD   dwFourCC			;//�t�H�[�}�b�g�� FourCC �ŕ\�������ꍇ�Ɏg�p����B DX10 �g���w�b�_�����݂���ꍇ�� 'DX10' (0x30315844) ������B
	DWORD   dwRGBBitCount		;// 1 pixel �� bit ��
	DWORD   dwRBitMask			;// RGB format ���� mask
	DWORD   dwGBitMask			;// RGB format ���� mask
	DWORD   dwBBitMask			;// RGB format ���� mask
	DWORD   dwRGBAlphaBitMask	;// RGB format ���� mask
	DWORD   dwCaps				;// mipmap ���̃t���O�w��p
	DWORD   dwCaps2				;// cube/volume texture ���̃t���O�w��p
	DWORD   dwReservedCaps[2];
	DWORD   dwReserved2;
};
enum {
	DDSD_CAPS			= 0x00000001,	// dwCaps/dwCpas2 ���L��
	DDSD_HEIGHT			= 0x00000002,	// dwHeight ���L��
	DDSD_WIDTH			= 0x00000004,	// dwWidth ���L���B���ׂẴe�N�X�`���ŕK�{�B
	DDSD_PITCH			= 0x00000008,	// dwPitchOrLinearSize �� Pitch ��\��
	DDSD_PIXELFORMAT	= 0x00001000,	// dwPfSize/dwPfFlags/dwRGB/dwFourCC�` ���� Pixel ��`���L���B
	DDSD_MIPMAPCOUNT	= 0x00020000,	// dwMipMapCount ���L���Bmipmap ���i�[����ꍇ�͕K�{�B
	DDSD_LINEARSIZE		= 0x00080000,	// dwPitchOrLinearSize �� LinearSize ��\��
	DDSD_DEPTH			= 0x00800000,	// dwDepth ���L���B3D (Volume) Texture ���ɐݒ�
};
enum {
	DDPF_ALPHAPIXELS	= 0x00000001,	// RGB �ȊO�� alpha ���܂܂�Ă���B�܂� dwRGBAlphaBitMask ���L���B
	DDPF_ALPHA			= 0x00000002,	// pixel �� Alpha �����̂݊܂܂�Ă���B
	DDPF_FOURCC			= 0x00000004,	// dwFourCC ���L���B
	DDPF_PALETTEINDEXED4= 0x00000008,	// * Palet 16 colors(DX9 �ȍ~�͂��Ԃ�g�p����Ȃ�)
	DDPF_PALETTEINDEXED8= 0x00000020,	// * Palet 256 colors(DX10 �ȍ~�͎g�p����Ȃ�)
	DDPF_RGB			= 0x00000040,	// dwRGBBitCount / dwRBitMask / dwGBitMask / dwBBitMask / dwRGBAlphaBitMask �ɂ���ăt�H�[�}�b�g����`����Ă��邱�Ƃ�����
	DDPF_LUMINANCE		= 0x00020000,	// * 1ch �̃f�[�^�� RGB ���ׂĂɓW�J�����
	DDPF_BUMPDUDV		= 0x00080000,	// * pixel �������t�ł��邱�Ƃ�����(�{���� bump �p) DX10�ȍ~�͎g�p���Ȃ�
};
enum {
	DDSCAPS_ALPHA		= 0x00000002,	// Alpha ���܂܂�Ă���ꍇ(���܂�Q�Ƃ���Ȃ�)
	DDSCAPS_COMPLEX		= 0x00000008,	// �����̃f�[�^���܂܂�Ă���ꍇ�BPalette / Mipmap / Cubemap / VolumeTexture �ł� On �ɂ���B
	DDSCAPS_TEXTURE		= 0x00001000,	// ��� On
	DDSCAPS_MIPMAP		= 0x00400000,	// MipMap �����݂���ꍇ�B(dwFlags �� DDSD_MIPMAPCOUNT �� On �ł��� dwMipMapCount �� 2�ȏ�̏ꍇ�� On)
};
enum {
	DDSCAPS2_CUBEMAP			= 0x00000200 ,//	Cubemap �̏ꍇ
	DDSCAPS2_CUBEMAP_POSITIVEX	= 0x00000400 ,//	Cubemap + X ���܂܂�Ă���
	DDSCAPS2_CUBEMAP_NEGATIVEX	= 0x00000800 ,//	Cubemap - X ���܂܂�Ă���
	DDSCAPS2_CUBEMAP_POSITIVEY	= 0x00001000 ,//	Cubemap + Y ���܂܂�Ă���
	DDSCAPS2_CUBEMAP_NEGATIVEY	= 0x00002000 ,//	Cubemap - Y ���܂܂�Ă���
	DDSCAPS2_CUBEMAP_POSITIVEZ	= 0x00004000 ,//	Cubemap + Z ���܂܂�Ă���
	DDSCAPS2_CUBEMAP_NEGATIVEZ	= 0x00008000 ,//	Cubemap - Z ���܂܂�Ă���
	DDSCAPS2_VOLUME				= 0x00400000 ,//	VolumeTexture �̏ꍇ
} ;

int		TsImageCore::Load(const FString& fname, EImageFile fmt) {
	FString	path = TsUtil::GetDirectory( fname ) ;
	FILE* fp;
	if ((_tfopen_s(&fp, *path, TEXT("rb"))) == 0) {
		switch (fmt) {
		case EImageFile::Bmp:{
				BITMAPFILEHEADER	bm_head;
				BITMAPINFOHEADER	bm_info;
				fread(&bm_head.bfType     , sizeof(char ), 2, fp);
				fread(&bm_head.bfSize     , sizeof(int  ), 1, fp);
				fread(&bm_head.bfReserved1, sizeof(short), 1, fp);
				fread(&bm_head.bfReserved2, sizeof(short), 1, fp);
				fread(&bm_head.bfOffBits  , sizeof(int  ), 1, fp);
				fread(&bm_info, sizeof(bm_info), 1, fp);
				mW = bm_info.biWidth;
				mH = bm_info.biHeight;
				//fread(mImage, sizeof(char), bm_info.biSizeImage, fp);
				fclose(fp);
			}
			break;

		case EImageFile::Png: {
				PNGTYPE       png_type;
				PNGHEADER     png_header;
				PNGChunkBegin png_chunk_begin;
				PNGChunkEnd   png_chunk_end;

				fread(&png_type, sizeof(png_type), 1, fp);
				fread(&png_header, sizeof(png_header), 1, fp);
				bool loading = true;
				while (loading) {
					fread(&png_chunk_begin, sizeof(png_chunk_begin), 1, fp);
					switch (png_chunk_begin.pnChunkType) {
					case 0x49444154://IDAT
						break;
					case 0x504C5445://PLTE  palette
					case 0x49454E44://IEND
						loading = false;
						break;
					}
					fread(&png_chunk_end, sizeof(png_chunk_end), 1, fp);
				}
			}
			break;
		}
	}
	return 0;
}


int		TsImageCore::Save(const FString& fname, EImageFile filetype, EImageFormat format, int x, int y, int w, int h ) const {
	if (w == 0) w = mW;
	if (h == 0) h = mH;
	UE_LOG(LogTemp, Log, TEXT("        Save[%hs]    xy=(%d,%d) w,h=%d %d"), TCHAR_TO_UTF8(*fname), x, y, w, h);

	if (format & FmtDebug) {
		UKismetSystemLibrary::PrintString(nullptr, fname, true, true, FColor::Red, 5.0f, TEXT("None"));
	}

	FString	path = TsUtil::GetDirectory( fname ) ;
	FILE* fp;
	if ((_tfopen_s(&fp, *path, TEXT("wb"))) == 0) {
		switch (filetype) {
		case EImageFile::Raw: {
				SaveImage(fp, format, x, y, w, h);
			}
			break;

		case EImageFile::Bmp:{
				int imgoffs = 14 + sizeof(BITMAPINFOHEADER);
				int imgbcc = GetStride(format) * 8;
				int imgsize = w * h * (imgbcc / 8);
				BITMAPFILEHEADER	bm_head = { {'B','M'}, imgoffs + imgsize, 0, 0, imgoffs };
				BITMAPINFOHEADER	bm_info = { sizeof(BITMAPINFOHEADER), w, h , 1, imgbcc, 0, imgsize, 0,0,0,0 };
				fwrite(&bm_head.bfType     , sizeof(UINT8 ), 2, fp);
				fwrite(&bm_head.bfSize     , sizeof(UINT32), 1, fp);
				fwrite(&bm_head.bfReserved1, sizeof(UINT16), 1, fp);
				fwrite(&bm_head.bfReserved2, sizeof(UINT16), 1, fp);
				fwrite(&bm_head.bfOffBits  , sizeof(UINT32), 1, fp);
				fwrite(&bm_info, sizeof(bm_info), 1, fp);
				SaveImage(fp, format,x,y,w,h);
				//fwrite(mImage, sizeof(UINT8), bm_info.biSizeImage, fp);
				UE_LOG(LogTemp, Log, TEXT("SaveFile done image  size=%d  BMPHEAD=%d   BMPINFO=%d  ImgOff=%d"), bm_info.biSizeImage, sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER), imgoffs);
			}
			break;
		case EImageFile::Dds: {
				int imgsize = w * h * GetStride(format);
				DDSHEADER	ds_header = { 0x20534444, 124,
										DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT, w, h, GetStride(format) * h, 1,
										0, {0}, 32, 0,0, GetStride(format) * 8,  0,0,0,0, DDSCAPS_TEXTURE, 0, {0} ,0 };
				switch (format & EImageFormat::FmtMask) {
				case EImageFormat::FormatB8G8R8A8:
					ds_header.dwPfFlags			= DDPF_RGB| DDPF_ALPHAPIXELS;
					ds_header.dwRBitMask		= 0x00ff0000;
					ds_header.dwGBitMask		= 0x0000ff00;
					ds_header.dwBBitMask		= 0x000000ff;
					ds_header.dwRGBAlphaBitMask	= 0xff000000;
					break;
				case EImageFormat::FormatG16R16:
					ds_header.dwPfFlags  = DDPF_RGB ;
					ds_header.dwRBitMask = 0x0000ffff;
					ds_header.dwGBitMask = 0xffff0000;
					break;
				case EImageFormat::FormatR32:// unspported for dds
				case EImageFormat::FormatR16:
					return -1;
				case EImageFormat::FormatR8:
					ds_header.dwPfFlags  = DDPF_LUMINANCE;
					ds_header.dwRBitMask = 0x000000ff;
					break;
				case EImageFormat::FormatL16:
					ds_header.dwPfFlags = DDPF_LUMINANCE;
					ds_header.dwRBitMask = 0x0000ffff;
					break;
				case EImageFormat::FormatF32:
					ds_header.dwPfFlags = DDPF_FOURCC;
					ds_header.dwFourCC	= 0x00000072  ;
					break;
				}
				fwrite(&ds_header, sizeof(DDSHEADER), 1, fp);
				SaveImage(fp, format, x, y, w, h);
			}
			break;

		case EImageFile::Png:{
				int imgsize  = w * h * GetStride(format);
				PNGTYPE			pn_type = { {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A} };
				PNGChunkBegin	pn_head_chunk = { 13     , 0x49484452 };
				PNGHEADER		pn_header     = { w, h, GetStride(format), 0,32,0,0,0 };
				PNGChunkBegin	pn_data_chunk = { imgsize, 0x49444154 };
				PNGChunkBegin	pn_end_chunk  = { 0      , 0x49454E44 };
				PNGChunkEnd		pn_crc	= { 0 };

				switch (format & EImageFormat::FmtMask) {
				case EImageFormat::FormatB8G8R8A8:
					pn_header.pnColorType = 6;
					pn_header.pnDepth = GetStride(format) * 8;
					break;
				case EImageFormat::FormatB8G8R8:
					pn_header.pnColorType = 2;
					pn_header.pnDepth = GetStride(format) * 8;
					break;
				case EImageFormat::FormatR8:
				case EImageFormat::FormatR16:
				case EImageFormat::FormatR32:
					pn_header.pnColorType = 0;
					pn_header.pnDepth = GetStride(format) * 8 ;
					break;
				case EImageFormat::FormatF32:
					return -1;
				}

				fwrite(&pn_type, sizeof(UINT8), 8, fp);
				
				fwrite(&pn_head_chunk, sizeof(PNGChunkBegin), 1, fp);
				fwrite(&pn_header.pnWidth       , sizeof(UINT32), 1, fp);
				fwrite(&pn_header.pnHeight      , sizeof(UINT32), 1, fp);
				fwrite(&pn_header.pnDepth       , sizeof(UINT8 ), 1, fp);
				fwrite(&pn_header.pnColorType   , sizeof(UINT8 ), 1, fp);
				fwrite(&pn_header.pnCompressType, sizeof(UINT8 ), 1, fp);
				fwrite(&pn_header.pnFilterType  , sizeof(UINT8 ), 1, fp);
				fwrite(&pn_header.pnInteraceType, sizeof(UINT8 ), 1, fp);
				fwrite(&pn_crc, sizeof(PNGChunkEnd), 1, fp);

				fwrite(&pn_data_chunk, sizeof(PNGChunkBegin), 1, fp);
				SaveImage(fp, format, x, y, w, h);
				//fwrite(mImage, sizeof(UINT8), imgsize, fp);
				fwrite(&pn_crc, sizeof(PNGChunkEnd), 1, fp);

				fwrite(&pn_end_chunk , sizeof(PNGChunkBegin), 1, fp);
				fwrite(&pn_crc, sizeof(PNGChunkEnd), 1, fp);
			}
			break;
		}

		fclose(fp);
	}
	return 0;
}


UTexture2D*		TsImageCore::SaveAsset(const FString& asset_name, EImageFormat format )  
{
	UTexture2D* tex = nullptr ;
#if WITH_EDITOR
    FString     package_path = TsUtil::GetPackagePath( asset_name ) ;
    FString     package_name = FPackageName::ObjectPathToPackageName( package_path ) ;
    UPackage*	package = CreatePackage( *package_name ) ;
    if (!package) return nullptr;

    // Texture2D �̐V�K�쐬
    tex = NewObject<UTexture2D>( package, *asset_name, RF_Public | RF_Standalone );
    if (!tex) return nullptr;

	bool						srgb;
	TextureCompressionSettings	tcs ;
	ETextureSourceFormat		tsf ;
	switch( format ){
	case FormatR8:		tsf = TSF_G8	 ; tcs = TC_Grayscale; srgb = false ; break ;
	case FormatR16:
	case FormatL16:		tsf = TSF_G16	 ; tcs = TC_Grayscale; srgb = false ; break ;
	case FormatF32:		tsf = TSF_R32F	 ; tcs = TC_Grayscale; srgb = false ; break ;
	case FormatB8G8R8:
	case FormatB8G8R8A8:tsf = TSF_BGRA8  ; tcs = TC_Default  ; srgb = true  ; break ;
	default:			tsf = TSF_Invalid; tcs = TC_Default  ; srgb = true  ; break ;
	}

	SlotSetup( format ) ;
	SlotConvert( format ) ;

    tex->Source.Init( mW,mH,mD, 1, tsf);

	uint8* data = tex->Source.LockMip(0);
	FMemory::Memcpy( data, SlotData(format), mW * mH );
	tex->Source.UnlockMip(0) ;

    // �ݒ�̒���
    tex->SRGB					= srgb ; // ���`���
    tex->CompressionSettings	= tcs  ;
    tex->MipGenSettings			= TMGS_NoMipmaps;
    tex->Filter					= TF_Default;
    tex->UpdateResource();

    // �A�Z�b�g�o�^�ƕۑ�
    FAssetRegistryModule::AssetCreated(tex);
    package->MarkPackageDirty();

    const FString pkg_filename = FPackageName::LongPackageNameToFilename(package_name, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs save_args;
    save_args.TopLevelFlags = RF_Public | RF_Standalone;
    save_args.SaveFlags = SAVE_NoError;
    save_args.bWarnOfLongFilename = true;
    UPackage::SavePackage(package, tex, *pkg_filename, save_args);

	SlotClear( format ) ;
#endif
    return tex;
}




int			TsImageCore::GetStride(EImageFormat format) {
	switch (format & EImageFormat::FmtMask) {
	case EImageFormat::FormatB8G8R8A8:
	case EImageFormat::FormatB8G8R8:	// no alpha
	case EImageFormat::FormatG16R16:
	case EImageFormat::FormatR32:
	case EImageFormat::FormatF32:	return 4;
	case EImageFormat::FormatL16:
	case EImageFormat::FormatR16:	return 2;
	case EImageFormat::FormatR8:	return 1;
	}
	return 0;
}

void		TsImageCore::SetWorld(const FBox2D* bound) 
{
	FVector2D	size = bound->GetSize();
	mStep = FVector2D( size.X/mW, size.Y/mH );
	mWorld = bound;
}

FVector2D	TsImageCore::GetWorldPos(int px, int py) const
{
	return FVector2D(mStep.X * px, mStep.Y * py) + mWorld->Min;
}

FVector2D	TsImageCore::GetTexcoord(const FVector2D &p ) const
{
	FVector2D lc = p - mWorld->Min;
	return FVector2D( lc.X/mStep.X, lc.Y/mStep.Y ) ;
}

FIntVector2	TsImageCore::GetPixelPos(const FVector2D &p ) const
{
	FVector2D tc = GetTexcoord( p );
	return FIntVector2( (int)(tc.X+0.5f), (int)(tc.Y+0.5f) ) ;
}

bool		TsImageCore::IsWorld(const FVector2D& p) const
{
	return mWorld->IsInside(p);
}

void		TsImageCore::ForeachPixel( std::function< void(int,int) > func, int inc )
{
	for (int py = 0; py < mH ; py += inc) {
		for (int px = 0; px < mW ; px += inc) {
			func( px, py ) ;
		}
	}
}



void		TsImageCore::SlotSetup( EImageFormat format )
{
	switch ( format & EImageFormat::FmtMask ) {
	case EImageFormat::FormatF32:	mSlot = new float  [mW*mH] ; break;
	case EImageFormat::FormatL16:
	case EImageFormat::FormatR16:	mSlot = new uint16 [mW*mH] ; break;
	case EImageFormat::FormatR8:	mSlot = new uint8  [mW*mH] ; break;
	default:						mSlot = nullptr ; break;
	}
}

void *		TsImageCore::SlotData( EImageFormat format ) const 
{
	switch ( format & EImageFormat::FmtMask ) {
	case EImageFormat::FormatF32:
	case EImageFormat::FormatL16:
	case EImageFormat::FormatR16:
	case EImageFormat::FormatR8:
		return mSlot;
	}
	return nullptr ;
}

void		TsImageCore::SlotClear( EImageFormat format )
{
	switch ( format & EImageFormat::FmtMask ) {
	case EImageFormat::FormatF32:	delete [] (float *)mSlot ;
	case EImageFormat::FormatL16:
	case EImageFormat::FormatR16:	delete [] (uint16*)mSlot ;
	case EImageFormat::FormatR8:	delete [] (uint8 *)mSlot ;
	}
	mSlot = nullptr ; 
}


// -------------------------------- TsImageMap  --------------------------------

int			TsImageMap<int>::SaveImage(FILE* fp, EImageFormat format, int sx, int sy, int w, int h) const
{
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int px = (format & EImageFormat::FmtHFlip) ? (sx + w-1-x) : (sx + x);
			int py = (format & EImageFormat::FmtVFlip) ? (sy + h-1-y) : (sx + y);
			int		v = GetPixel(px, py);
			float	f;
			switch (format & EImageFormat::FmtMask) {
			case EImageFormat::FormatF32:
				f = v;
				fwrite(&f, GetStride(format), 1, fp);
				break;
			case EImageFormat::FormatG16R16:
			case EImageFormat::FormatL16:
			case EImageFormat::FormatR16:
			case EImageFormat::FormatB8G8R8A8:
			case EImageFormat::FormatR8:
				if (format & EImageFormat::FmtDebug) UE_LOG(LogTemp, Log, TEXT("(%d,%d) %08x"), x, y, v);
				fwrite(&v, GetStride(format), 1, fp);
				break;
			}
		}
	}
	return 0;
}


int			TsImageMap<float>::SaveImage(FILE* fp, EImageFormat format, int sx, int sy, int w, int h) const
{
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int px = (format & EImageFormat::FmtHFlip) ? (sx + w - 1 - x) : (sx + x);
			int py = (format & EImageFormat::FmtVFlip) ? (sy + h - 1 - y) : (sx + y);
			float f = GetPixel(px, py) ;

			switch (format & EImageFormat::FmtMask) {
			case EImageFormat::FormatF32:
				if (format & EImageFormat::FmtDebug) UE_LOG(LogTemp, Log, TEXT("(%d,%d) %f[%f]"), x, y, f, RemapImage(f, 1));
				f = RemapImage( f, 1 );
				fwrite(&f, GetStride(format), 1, fp);
				break;

			default:
				int v=0;
				switch (format & EImageFormat::FmtMask) {
				case EImageFormat::FormatG16R16:
				case EImageFormat::FormatL16:
				case EImageFormat::FormatR16:
					v = (int)RemapImage(f, 65535);
					break;
				case EImageFormat::FormatB8G8R8A8:
					v = (int)RemapImage(f, 255) | 0xff000000;
					break;
				case EImageFormat::FormatR8:
					v = (int)RemapImage(f, 255);
					break;
				default:
					break;
				}
				if (v<0) v=0 ;
				if (format & EImageFormat::FmtDebug){
					 UE_LOG(LogTemp, Log, TEXT("(%d,%d) %f[%d]"), x, y, f, v);
				}

				fwrite(&v, GetStride(format), 1, fp);
				break;
			}
		}
	}
	return 0;
}

