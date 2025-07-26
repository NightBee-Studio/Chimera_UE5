#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "TsUtility.h"


// -------------------------------- TsMapOutput  --------------------------------

struct TsMapOutput {
	int				mX, mY;		// start offset ( mX, mY )*local_reso
	int				mDiv;		// division
	int				mWorldReso;	// world 
	const FBox2D *	mWorldBound;

	TsMapOutput() : mX(0), mY(0), mDiv(1), mWorldReso(512), mWorldBound(nullptr) {}
	TsMapOutput(int _x, int _y, int _n, int world_reso, const FBox2D *world_bound	)
		: mX(_x), mY(_y)
		, mDiv(_n)
		, mWorldReso(world_reso), mWorldBound(world_bound) {}

	FBox2D			LocalBound( int x, int y, int reso );
	TsUtil::TsBox	TexBound( int x, int y, int reso ) ;
};



// -------------------------------- TsValueMap  --------------------------------
class TsValueMap
{
protected:
	public:
	float			mMin, mMax;
public:
	TsValueMap()				{ ResetRemap(); }

	float			GetGap()	{ return mMax - mMin; }

	virtual void	ResetRemap();
	virtual float	Remap(float val) const ;
	void			RemapDone();
	virtual void	UpdateRemap(const FVector2D& p);

	virtual float	GetValue(const FVector2D& p) = 0;	// world-coord
};



// -------------------------------- TsNoiseMap  --------------------------------
struct TsNoiseParam {
	FVector2D			mNoisePos;
	TArray<FVector2D>	mLayers;
	TsNoiseParam()
		: mNoisePos(0,0), mLayers(){}
	TsNoiseParam( std::initializer_list<FVector2D> layers, int seed = -1)
		: mNoisePos(0,0), mLayers(layers) { Setup(seed); }
	void Setup( int seed = -1 ) ;
};

class TsNoiseMap
	: public TsValueMap
	, public TsNoiseParam
{
public:
	TsNoiseMap(const TsNoiseParam& cnf) : TsValueMap(), TsNoiseParam(cnf) {}
	TsNoiseMap() : TsValueMap(), TsNoiseParam() {}
	virtual ~TsNoiseMap() {}

	virtual float	GetValue(const FVector2D& p) override;		// world-coord
};







// -------------------------------- TsImageMap  --------------------------------
enum EImageFormat {
	FormatInvalid = 0,
	FormatB8G8R8A8,
	FormatB8G8R8,
	FormatF32,
	FormatG16R16,
	FormatR8,
	FormatR16,
	FormatL16,
	FormatR32,
	FmtMask  = 0x0000ffff,
	FmtDebug = 0x10000000,
	FmtVFlip = 0x20000000,
	FmtHFlip = 0x40000000,

	FormatR8_Debug = FormatR8 | FmtDebug,
	FormatL16_VFlip = FormatL16 | FmtVFlip,
	FormatB8G8R8A8_Debug = FormatB8G8R8A8 | FmtDebug,
	FormatL16_Debug = FormatL16 | FmtDebug,
};


enum EImageMode {
	Clamp,
	Wrap,
};


enum EImageFile{
	Bmp,
	Png,
	Dds,
	Raw,
};

class UTexture2D ;

class TsImageCore {
protected:
	int				mW, mH, mD;
	void*			mImage;

	const FBox2D *	mWorld;
	FVector2D		mStep;

	static FString	gDirName;
	FString			mFileName;
	EImageFile		mFileType;
	EImageFormat	mFileFormat;

	TsMapOutput		mConfig;

public:
	enum EOp {
		Set,
		Add,
		Max,
	};

public:
	TsImageCore(int w, int h, int d, const FString & name)
		: mW(w), mH(h), mD(d)
		, mImage(nullptr)
		, mWorld(nullptr)
		, mFileName(name)
		, mFileType(EImageFile::Dds)
		, mFileFormat(EImageFormat::FormatL16)
		, mConfig() {}
	virtual			~TsImageCore() {}

	bool			IsInside(int x, int y) const { return (x >= 0 && x < mW) && (y >= 0 && y < mH) ; }
	static int		GetStride(EImageFormat format) ;
	int				GetW() const { return mW; }
	int				GetH() const { return mH; }
	float			GetStep() const { return mStep.X; }

public:
	bool			IsWorld(const FVector2D& p) const;
	void			SetWorld(const FBox2D* bound);
	const FBox2D*	GetWorld() const { return mWorld; }
	FVector2D		GetWorldPos(int px, int py) const ;		// tex-coord -> world-coord
	FIntVector2		GetPixelPos(const FVector2D& p) const;	// world-coord -> tex-coord
	void			SetMapping(const TsMapOutput& conf) { mConfig = conf; }

	void			ForeachPixel( std::function< void(int, int) >, int inc=1 );
	
private:
	virtual void*	ConvertImage( EImageFormat format ) const { return nullptr ;}
	virtual int		SaveImage(FILE* fp, EImageFormat format, int x, int y, int w, int h) const { return  0; }
public:
	virtual	float	RemapImage(float v, float range = 1.0f) const { return v * range; }

//	static void		SetDirectory(const FString& path, int no_x=-1, int no_y=-1);

	int				Load(const FString& fname, EImageFile file) ;
	int				Save(const FString& fname, EImageFile file, EImageFormat format,int x=0, int y=0,int w=0, int h=0 ) const;
	UTexture2D*		SaveAsset(const FString& fname, EImageFormat format ) const;
	int				Load()		{ return Load(mFileName, mFileType             ); }
	int				Save() const{ return Save(mFileName, mFileType, mFileFormat,0,0,mW,mH); }
};


template<typename T>
class TsImageMap : public TsImageCore {
public:
	TsImageMap(int w, int h, int d, const FBox2D* bound = nullptr, const FString& name = "Image00")
		: TsImageCore(w, h, d, name) {
		if (bound != nullptr) {
			SetWorld(bound);
			AllocImage();
		}
	}
	TsImageMap(int w, int h, const FBox2D* bound=nullptr, const FString& name = "Image00")
		: TsImageCore(w, h, 1, name) {
		if (bound != nullptr ){
			SetWorld(bound);
			AllocImage();
		}
	}
	virtual ~TsImageMap() {}

	void 		ClearImage() {
		FMemory::Memzero(mImage, sizeof(T) * mW * mH);
	}
	void		AllocImage() {
		if (mImage == nullptr) {
			mImage = FMemory::Malloc(sizeof(T) * mW * mH);
			ClearImage();
		}
	}
	void 		ReleaseImage() {
		if (mImage != nullptr) {
			FMemory::Free(mImage);
			mImage = nullptr;
		}
	}
	
	void*		ConvertImage( EImageFormat format ) const override{
		switch (format & EImageFormat::FmtMask) {
		case EImageFormat::FormatF32:{
				TArray<float> data ;
				for (int i = 0; i < mW*mH*mD; i++ ) data.Add( (const float)((T*)mImage)[i] ) ;
				return data.GetData() ;
			}
			break;
		case EImageFormat::FormatL16:
		case EImageFormat::FormatR16:{
				TArray<uint16> data ;
				for (int i = 0; i < mW*mH*mD; i++ ) data.Add( (const uint16)(((T*)mImage)[i]*65535) ) ;
				return data.GetData() ;
			}
			break;
		case EImageFormat::FormatR8:{
				TArray<uint8> data ;
				for (int y = 0; y < mH ; y++ ){
					for (int x = 0; x < mW ; x++ ){
						data.Add( (const uint8)(GetPixel(x,y)*255) ) ;
					}
				}
				return data.GetData() ;
			}
			break;
		default:{
				UE_LOG(LogTemp, Log, TEXT("Invalid Format %d"), format );
			}
			return nullptr ;;
		}
	}

	int			SaveImage(FILE* fp, EImageFormat format, int sx, int sy, int w, int h ) const override;
	T*			GetImage( void ) const { return (T*)mImage ;}

	T			GetPixel( const FVector2D& p, EImageMode mode = EImageMode::Clamp) const {		// world-coord -> tex-coord
		FIntVector2	pix = GetPixelPos(p);
		return	GetPixel(pix.X, pix.Y, mode);
	}
	T			GetPixel(int x, int y, EImageMode mode=EImageMode::Clamp ) const {
		int dx, dy;
		switch (mode) {
		case EImageMode::Wrap:
			dx = (x < 0 ? mW-x : x) % mW ;
			dy = (y < 0 ? mH-y : y) % mH ;
			break;
		default:
			dx = x < 0 ? 0 : x >= mW ? mW - 1 : x;
			dy = y < 0 ? 0 : y >= mH ? mH - 1 : y;
			break;
		}
		return ((T*)mImage)[dx + mW * dy];
	}

	virtual void	SetPixel(const FVector2D& p, T v) {	// world-coord -> tex-coord
		FIntVector2	pix = GetPixelPos(p);
		SetPixel(pix.X, pix.Y, v);
	}
	virtual void	SetPixel(int x, int y, T v) {
		int dx = x < 0 ? 0 : x >= mW ? mW - 1 : x;
		int dy = y < 0 ? 0 : y >= mH ? mH - 1 : y;
		((T*)mImage)[dx + mW * dy] = v;
	}

	void		DrawLine(	int x0, int y0, T v0,
							int x1, int y1, T v1, 
							EOp op=EOp::Set) {
		float	dx = x1 - x0;
		float	dy = y1 - y0;
		T		dv = v1 - v0;
		float step = fabsf( fabsf(dx)<fabsf(dy) ? dy : dx );
		dx /= step;
		dy /= step;
		dv /= step;

		float	x = x0;
		float	y = y0;
		T		v = v0;
		for (int i=1; i<=step ; i++){
			T val ;
			switch (op) {
			case EOp::Max:
				val = FMath::Max(GetPixel((int)x, (int)y), v);
				break;
			case EOp::Add:
				val = GetPixel((int)x, (int)y) + v;
				break;
			case EOp::Set:
			default:
				val = v;
				break;
			}
			SetPixel((int)x, (int)y, val );
			x += dx;
			y += dy;
			v += dv;
		}
	}
};

