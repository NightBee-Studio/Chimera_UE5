#pragma once

// ---------------------------- Utillity -------------------------
namespace TsUtil {
	static inline
	FVector2D	NearPoint(const FVector2D& v1, const FVector2D& v2, const FVector2D& pp)
	{
		FVector2D ap = pp - v1;
		FVector2D ab = v2 - v1;
		if (ap.Dot(ab) < 0) return v1;
		FVector2D bp = pp - v2;
		FVector2D ba = v1 - v2;
		if (bp.Dot(ba) < 0) return v2;
		return v1 + ab / ab.Length() * (ap.Dot(ab) / ab.Length());
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
}
