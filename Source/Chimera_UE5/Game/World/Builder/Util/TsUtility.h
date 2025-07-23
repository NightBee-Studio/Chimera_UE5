#pragma once

#include <functional>

#include <vector>
#include <cmath>

// ---------------------------- Utillity -------------------------
namespace TsUtil {
	struct TsBox{
		int		mX, mY ; 
		int		mW, mH ; 

		TsBox( int x, int y, int w, int h )
			: mX(x),mY(y), mW(w),mH(h){}
	};

	static inline
	FVector2D	NearPoint(const FVector2D& v1, const FVector2D& v2, const FVector2D& pp)
	{
		FVector2D ap = pp - v1;
		FVector2D ab = v2 - v1;
		float     ad = ap.Dot(ab);
		if (ad < 0) return v1;
		FVector2D bp = pp - v2;
		FVector2D ba = v1 - v2;
		float     bd = bp.Dot(ba);
		if (bd < 0) return v2;

//		UE_LOG(LogTemp, Log, TEXT("NearPoint Dot%f = %f"), ad, ad / ab.Length());

		return v1 + ab / ab.Length() * (ad / ab.Length());
	}

	static inline
	FVector2D	LinePoint(const FVector2D& lp, const FVector2D& ld, const FVector2D& pp)
	{
		float L = ld.Length();
		if (L <= 0.0000001f)return lp;

		FVector2D dir = ld / L ;		// 単位方向ベクトル
		FVector2D v   = pp - lp;		// 始点から点までのベクトル
		float     t   = FMath::Clamp(v.Dot(dir), 0.0f, L);// スカラー投影（垂線の足までの距離）

		return lp + dir * t;		// 最も近い点（線分上）


		//FVector2D v = pp - lp;
		//float     l = ld.Length();
		//if (l==0.0f) return lp;
		//float     d = FMath::Clamp( v.Dot(ld), 0, l );

		////UE_LOG(LogTemp, Log, TEXT("NearPoint Dot%f"), d  );

		//return lp + ld/l * d ;
	}

	static inline
	void		ForeachGaussian( const FVector2D &p, float pixel, std::function<void( const FVector2D& p, float weight )> func) {
		static float w25[5][5] = {
			{1, 4, 7, 4,1},
			{4,16,26,16,4},
			{7,26,41,26,7},
			{4,16,26,16,4},
			{1, 4, 7, 4,1},
		};
		for (int y = 0; y < 5; y++) {
			for (int x = 0; x < 5; x++) {
				func( p + FVector2D(x-2, y-2) * pixel, w25[x][y] / 273.0f );
			}
		}
	}

	static inline
		void ForeachGaussianEx(
			const FVector2D& p,
			float pixel,
			int size,
			std::function<void(const FVector2D& p, float weight)> func)
	{
		// サイズは奇数である必要があります（例: 3, 5, 7,...）
		if (size % 2 == 0 || size < 1) size++;

		// カーネルの半径
		int   radius = size / 2;
		float sigma = size / 3.0f; // σをカーネルサイズから推定
		float sig_sq = 2.0f * sigma * sigma;
		float sum = 0.0f;

		// 2Dカーネルを生成
		std::vector<std::vector<float>> kernel(size, std::vector<float>(size));
		for (int y = -radius; y <= radius; ++y) {
			for (int x = -radius; x <= radius; ++x) {
				float r2 = static_cast<float>(x * x + y * y);
				float value = std::exp(-r2 / sig_sq);
				kernel[y + radius][x + radius] = value;
				sum += value;
			}
		}

		for (int y = 0; y < size; ++y) {
			for (int x = 0; x < size; ++x) {
				FVector2D offset(x - radius, y - radius);
				func(p + offset * pixel, kernel[y][x] / sum );
			}
		}
	}

	static inline FVector2D SinCosPos(float ang) {
		float x, y;
		FMath::SinCos(&y, &x, FMath::DegreesToRadians(ang));
		return FVector2D(x, y);
	}


	// ID 
	static int GenPosID(float x, float y, float dx = 0, float dy = 0) {
		unsigned int code = 0x0fffffff & (unsigned int)(x + 100 * y + dx + dy * 20);
		return (int)code + 1;
	}

	static float Sigmoid(float x) {
		return tanh(x / 2);
		//	return (tanh(x/2) + 1)/2 - 0.5f ;
	}


	// Random
	void	RandSeed(int i);
	int32	RandRange(int32 a, int32 b);
	float	RandRange(float a, float b);
	float	RandRange(const FVector2D &v);


	// directory
	void		SetDirectory(const FString& path, int no_x=-1, int no_y=-1);
	FString 	GetDirectory(const FString& name);
	FString 	GetPackagePath(const FString& name);

}
