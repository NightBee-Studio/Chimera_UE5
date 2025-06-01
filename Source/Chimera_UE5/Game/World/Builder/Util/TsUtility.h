#pragma once

#include <functional>

// ---------------------------- Utillity -------------------------
namespace TsUtil {
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
	void		ForeachGaussian( const FVector2D &p, float pixel, std::function<void( const FVector2D& p, float weight )> func ) {
		static float w[5][5] = {
			{1, 4, 7, 4,1},
			{4,16,26,16,4},
			{7,26,41,26,7},
			{4,16,26,16,4},
			{1, 4, 7, 4,1},
		};
		for (int y = 0; y < 5; y++) {
			for (int x = 0; x < 5; x++) {
				func( p + FVector2D(x-2, y-2) * pixel, w[x][y] / 273.0f );
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
	float	RandRange(float a, float b);
	int32	RandRange(int32 a, int32 b);
}
