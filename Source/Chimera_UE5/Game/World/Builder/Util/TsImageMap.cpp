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
	UINT32	biSize;//ヘッダサイズ	40 - Windows Bitmap	12 - OS/2 Bitmap
	UINT32	biWidth;//画像の幅 (ピクセル)
	UINT32	biHeight;//画像の高さ (ピクセル)
	INT16	biPlanes;//プレーン数　常に 1
	INT16	biBitCount;//1画素あたりのデータサイズ(bit)	例）256 色ビットマップ ＝ 8
	UINT32	biCompression;//圧縮形式	0:BI_RGB(無圧縮)	1:BI_RLE8 (RunLength 8 bits/pixel)	2:BI_RLE4 (RunLength 4 bits/pixel)	3:BI_BITFIELDS (Bitfields)	(4 - BI_JPEG)	(5 - BI_PNG)
	UINT32	biSizeImage;//画像データ部のサイズ(byte)
	UINT32	biXPelsPerMeter;//横方向解像度 (1mあたりの画素数)	//96dpi ならば3780
	UINT32	biYPelsPerMeter;//縦方向解像度 (1mあたりの画素数)	//96dpi ならば3780
	UINT32	biClrUsed;//格納されているパレット数 (使用色数)
	UINT32	biClrImportant;//重要なパレットのインデックス		//0 の場合もある
};


// Format                                    -------------------------------- PNG 
//
struct	PNGTYPE {	// BMP struct size will be 16, but Bmp-Format must be 14...
	UINT8    pnType[8];
};
struct	PNGHEADER {			// ChunkType =49 48 44 52	"IHDR" 
	UINT32	pnWidth;		//画像の幅 (ピクセル)
	UINT32	pnHeight;		//画像の高さ (ピクセル)
	UINT8	pnDepth;			//	ビット深度	(有効な値は 1, 2, 4, 8, 16 だが、
	UINT8	pnColorType;	//	カラータイプ(有効な値は 0, 2, 3, 4, 6)
	UINT8	pnCompressType;	//	圧縮手法
	UINT8	pnFilterType;	//	フィルター手法
	UINT8	pnInteraceType;	//インターレース手法
	UINT32	pnCRC;			// (Cyclic Redundancy Check)	Chunk Type と Chunk Data を もとに計算される
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
	DWORD   dwMagic				;// == 常に 0x20534444  ' SDD'
	DWORD   dwSize				;// == 常に 124
	DWORD   dwFlags				;// ヘッダ内の有効な情報 DDSD_* の組み合わせ
	DWORD   dwHeight			;// 画像の高さ x size
	DWORD   dwWidth				;// 画像の幅   y size
	DWORD   dwPitchOrLinearSize	;// 横1 line の byte 数 (pitch) または 1面分の byte 数 (linearsize)
	DWORD   dwDepth				;// 画像の奥行き z size (Volume Texture 用)
	DWORD   dwMipMapCount		;// 含まれている mipmap レベル数
	DWORD   dwReserved1[11];
	DWORD   dwPfSize			;// == 常に 32
	DWORD   dwPfFlags			;// pixel フォーマットを表す DDPF_* の組み合わせ
	DWORD   dwFourCC			;//フォーマットが FourCC で表現される場合に使用する。 DX10 拡張ヘッダが存在する場合は 'DX10' (0x30315844) が入る。
	DWORD   dwRGBBitCount		;// 1 pixel の bit 数
	DWORD   dwRBitMask			;// RGB format 時の mask
	DWORD   dwGBitMask			;// RGB format 時の mask
	DWORD   dwBBitMask			;// RGB format 時の mask
	DWORD   dwRGBAlphaBitMask	;// RGB format 時の mask
	DWORD   dwCaps				;// mipmap 等のフラグ指定用
	DWORD   dwCaps2				;// cube/volume texture 等のフラグ指定用
	DWORD   dwReservedCaps[2];
	DWORD   dwReserved2;
};
enum {
	DDSD_CAPS			= 0x00000001,	// dwCaps/dwCpas2 が有効
	DDSD_HEIGHT			= 0x00000002,	// dwHeight が有効
	DDSD_WIDTH			= 0x00000004,	// dwWidth が有効。すべてのテクスチャで必須。
	DDSD_PITCH			= 0x00000008,	// dwPitchOrLinearSize が Pitch を表す
	DDSD_PIXELFORMAT	= 0x00001000,	// dwPfSize/dwPfFlags/dwRGB/dwFourCC～ 等の Pixel 定義が有効。
	DDSD_MIPMAPCOUNT	= 0x00020000,	// dwMipMapCount が有効。mipmap を格納する場合は必須。
	DDSD_LINEARSIZE		= 0x00080000,	// dwPitchOrLinearSize が LinearSize を表す
	DDSD_DEPTH			= 0x00800000,	// dwDepth が有効。3D (Volume) Texture 時に設定
};
enum {
	DDPF_ALPHAPIXELS	= 0x00000001,	// RGB 以外に alpha が含まれている。つまり dwRGBAlphaBitMask が有効。
	DDPF_ALPHA			= 0x00000002,	// pixel は Alpha 成分のみ含まれている。
	DDPF_FOURCC			= 0x00000004,	// dwFourCC が有効。
	DDPF_PALETTEINDEXED4= 0x00000008,	// * Palet 16 colors(DX9 以降はたぶん使用されない)
	DDPF_PALETTEINDEXED8= 0x00000020,	// * Palet 256 colors(DX10 以降は使用されない)
	DDPF_RGB			= 0x00000040,	// dwRGBBitCount / dwRBitMask / dwGBitMask / dwBBitMask / dwRGBAlphaBitMask によってフォーマットが定義されていることを示す
	DDPF_LUMINANCE		= 0x00020000,	// * 1ch のデータが RGB すべてに展開される
	DDPF_BUMPDUDV		= 0x00080000,	// * pixel が符号付であることを示す(本来は bump 用) DX10以降は使用しない
};
enum {
	DDSCAPS_ALPHA		= 0x00000002,	// Alpha が含まれている場合(あまり参照されない)
	DDSCAPS_COMPLEX		= 0x00000008,	// 複数のデータが含まれている場合。Palette / Mipmap / Cubemap / VolumeTexture では On にする。
	DDSCAPS_TEXTURE		= 0x00001000,	// 常に On
	DDSCAPS_MIPMAP		= 0x00400000,	// MipMap が存在する場合。(dwFlags の DDSD_MIPMAPCOUNT が On でかつ dwMipMapCount が 2以上の場合に On)
};
enum {
	DDSCAPS2_CUBEMAP			= 0x00000200 ,//	Cubemap の場合
	DDSCAPS2_CUBEMAP_POSITIVEX	= 0x00000400 ,//	Cubemap + X が含まれている
	DDSCAPS2_CUBEMAP_NEGATIVEX	= 0x00000800 ,//	Cubemap - X が含まれている
	DDSCAPS2_CUBEMAP_POSITIVEY	= 0x00001000 ,//	Cubemap + Y が含まれている
	DDSCAPS2_CUBEMAP_NEGATIVEY	= 0x00002000 ,//	Cubemap - Y が含まれている
	DDSCAPS2_CUBEMAP_POSITIVEZ	= 0x00004000 ,//	Cubemap + Z が含まれている
	DDSCAPS2_CUBEMAP_NEGATIVEZ	= 0x00008000 ,//	Cubemap - Z が含まれている
	DDSCAPS2_VOLUME				= 0x00400000 ,//	VolumeTexture の場合
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

    // Texture2D の新規作成
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

    // 設定の調整
    tex->SRGB					= srgb ; // 線形空間
    tex->CompressionSettings	= tcs  ;
    tex->MipGenSettings			= TMGS_NoMipmaps;
    tex->Filter					= TF_Default;
    tex->UpdateResource();

    // アセット登録と保存
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

