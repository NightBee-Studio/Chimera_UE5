
#include "TsImageMap.h"

#include <stdio.h>

#include "CoreMinimal.h"

#include "GenericPlatform/GenericPlatformFile.h"
#include "Kismet/KismetSystemLibrary.h"





// -------------------------------- TsMapOutput  --------------------------------
int		TsMapOutput::LocalReso()
{
	return reso * (n>1 ? 2 : 1);
}

FBox2D	TsMapOutput::LocalBound(const FBox2D& world_bound)
{
	if (n == 1) return world_bound;

	FVector2D	size = world_bound.GetSize()/n;
	return FBox2D(	world_bound.Min + size * FVector2D( x-0.5f, y-0.5f ),
					world_bound.Min + size * FVector2D( x+1.5f, y+1.5f ) );
}




// -------------------------------- TsValueMap  --------------------------------
void	TsValueMap::ResetRemap() {
	mMin = 10000.0f;
	mMax = -10000.0f;
}

float	TsValueMap::Remap(float val) const {
	return  (mMax - mMin) > 0 ? (val - mMin) / (mMax - mMin) : 1;
}

void	TsValueMap::UpdateRemap(const FVector2D& p) {
	float h = GetValue(p);
	mMin = FMath::Min(mMin, h);
	mMax = FMath::Max(mMax, h);
}



// -------------------------------- TsNoiseMap  --------------------------------
void TsNoiseParam::Setup(int seed )
{
	if ( seed != -1) FMath::RandInit(seed);
	mNoisePos = FVector2D(FMath::RandRange(0, 1024), FMath::RandRange(0, 1024));
}

float	TsNoiseMap::GetValue(const FVector2D& p) {
	return	( mN0 * FMath::PerlinNoise2D(p * mS0 + mNoisePos)
			+ mN1 * FMath::PerlinNoise2D(p * mS1 + mNoisePos));
}



// -------------------------------- TsImageMap  --------------------------------

float	TsImageCore::gSx = 0;
float	TsImageCore::gSY = 0;

FString TsImageCore::gDirName;

#define PROJPATH	"D:\\Works\\Projects\\LandscapeGen\\Content\\"
#define DEMODIR		"Demo\\"
void	TsImageCore::SetDir(const FString& path, int no_x, int no_y)
{
	gDirName = PROJPATH DEMODIR + path;
	if (no_x >= 0 && no_y >= 0) {
		gDirName += FString::Printf(TEXT("%02d_%02d"), no_x, no_y);
	}
	gDirName += FString("\\");

	IPlatformFile& pf = FPlatformFileManager::Get().GetPlatformFile();
	if (!pf.DirectoryExists(*gDirName)) {		// Directory Exists?
		pf.CreateDirectory(*gDirName);
	}
}

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
struct	PNGChunkBegin {
	UINT32	pnLength;
	UINT32	pnChunkType;
};
struct	PNGChunkEnd {
	UINT32	pnCRC;
};


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
	FString	path = PROJPATH DEMODIR + fname;
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

		case EImageFile::Png:{
			}
			break;
		}
	}
	return 0;
}

int		TsImageCore::Save(const FString& fname, EImageFile filetype, EImageFormat format, int x, int y, int w, int h ) const {
	if (w == 0) w = mW;
	if (h == 0) h = mH;

	if (format & FmtDebug) {
		UKismetSystemLibrary::PrintString(nullptr, fname, true, true, FColor::Red, 5.0f, TEXT("None"));
	}

	UE_LOG(LogTemp, Log, TEXT("TsImageCore::Save xy=(%d,%d)   w,h=%d %d"), x, y, w, h);

	FString	path = gDirName + fname;
	FILE* fp;
	if ((_tfopen_s(&fp, *path, TEXT("wb"))) == 0) {
		switch (filetype) {
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
				fwrite(&pn_header.pnDepth       , sizeof(UINT8), 1, fp);
				fwrite(&pn_header.pnColorType   , sizeof(UINT8), 1, fp);
				fwrite(&pn_header.pnCompressType, sizeof(UINT8), 1, fp);
				fwrite(&pn_header.pnFilterType  , sizeof(UINT8), 1, fp);
				fwrite(&pn_header.pnInteraceType, sizeof(UINT8), 1, fp);
				fwrite(&pn_crc, sizeof(PNGChunkEnd), 1, fp);

				fwrite(&pn_data_chunk, sizeof(PNGChunkBegin), 1, fp);
				SaveImage(fp, format, x, y, w, h);
				//fwrite(mImage, sizeof(UINT8), imgsize, fp);
				fwrite(&pn_crc, sizeof(PNGChunkEnd), 1, fp);

				fwrite(&pn_end_chunk , sizeof(PNGChunkBegin), 1, fp);
				fwrite(&pn_crc, sizeof(PNGChunkEnd), 1, fp);
			//	UE_LOG(LogTemp, Log, TEXT("SaveFile done image  size=%d  BMPHEAD=%d   BMPINFO=%d  ImgOff=%d"), bm_info.biSizeImage, sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER), imgoffs);
			}
			break;
		}

		fclose(fp);
	}
	return 0;
}



int			TsImageCore::GetStride(EImageFormat format) {
	switch (format & EImageFormat::FmtMask) {
	case EImageFormat::FormatB8G8R8A8:
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
	mStep = FVector2D((size.X + 1) / mW, (size.Y + 1) / mH);
	mWorld = bound;
}

FVector2D	TsImageCore::GetWorldPos(int px, int py) const
{
	return FVector2D(mStep.X * px, mStep.Y * py) + mWorld->Min;
}

FIntVector2	TsImageCore::GetPixelPos(const FVector2D &p ) const
{
	FVector2D pix = p - mWorld->Min;
	//pix.X -= gSx;
	//pix.Y -= gSY;
	return FIntVector2( (int)(pix.X/mStep.X+0.5f), (int)(pix.Y/mStep.Y + 0.5f) ) ;
}

bool		TsImageCore::IsWorld(const FVector2D& p) const
{
	return mWorld->IsInside(p);
}

void	TsImageCore::ForeachPixel( std::function< void(int,int) > func )
{
	for (int py = 0; py < mH ; py++) {
		for (int px = 0; px < mW ; px++) {
			func( px, py ) ;
		}
	}
}

int		TsImageMap<int>::SaveImage(FILE* fp, EImageFormat format, int sx, int sy, int w, int h) const
{
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int		v = GetPixel(sx + x, sy + y);
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
				fwrite(&v, GetStride(format), 1, fp);
				break;
			}
		}
	}
	return 0;
}

int		TsImageMap<float>::SaveImage(FILE* fp, EImageFormat format, int sx, int sy, int w, int h) const
{
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			float f = GetPixel(sx+x, sy+y) ;

			if (format & EImageFormat::FmtDebug) UE_LOG(LogTemp, Log, TEXT("(%d,%d) %f[%d]"), x,y, f, (int)RemapImage(f, 65535));

			switch (format & EImageFormat::FmtMask) {
			case EImageFormat::FormatF32:
				f = RemapImage( f, 1 );
				fwrite(&f, GetStride(format), 1, fp);
				break;

			default:
				int v;
				switch (format & EImageFormat::FmtMask) {
				case EImageFormat::FormatG16R16:
				case EImageFormat::FormatL16:
				case EImageFormat::FormatR16:
					v = RemapImage(f, 65535);
					break;
				case EImageFormat::FormatB8G8R8A8:
					v = (int)RemapImage(f, 255) | 0xff000000;
					break;
				case EImageFormat::FormatR8:
					v = RemapImage(f, 255);
					break;
				default:
					break;
				}
				fwrite(&v, GetStride(format), 1, fp);
				break;
			}
		}
	}
	return 0;
}
