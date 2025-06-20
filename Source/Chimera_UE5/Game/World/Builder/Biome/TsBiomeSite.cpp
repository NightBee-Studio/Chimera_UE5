
#include "TsBiomeSite.h"
#include "../Util/TsUtility.h"




// ----------------------------- Circle ------------------------------
bool	TsBiomeSite::Circle::IsInside( const FVector2D& p ) {
	int cn = 0;											// the  crossing number counter
	for (int i = 0; i < n; i++) {						// loop through all edges of the polygon
		FVector2D a0 = GetOutline(i + 0);
		FVector2D a1 = GetOutline(i + 1);
		// upward crossing or downward crossing
		if (a0.Y > p.Y ? a1.Y <= p.Y : a1.Y > p.Y) {
			float vt = (p.Y - a0.Y) / (a1.Y - a0.Y);	// compute  the actual edge - ray intersect x - coordinate
			if (p.X < (a0.X + vt * (a1.X - a0.X))) {	// P.x < intersect - ray toward right - original
				cn++;									// a valid crossing of y = P[1] right of P.x
			}
		}
	}
	return (cn & 1) > 0;    // 0 if even(out), and 1 if  odd(in)
}

#define S 0.06f
FVector2D TsBiomeSite::Circle::GetOutline(int i) {
	float ang  = (i % n) * 360.0f / n;
	float bias = FMath::PerlinNoise3D( FVector(X, Y, ang * 0.06f) );
	float l = (1 + bias * 0.5f) * r;
	return *this + l * TsUtil::SinCosPos(ang);
}

#define N 128
void	TsBiomeSite::CreateChild(  const Circle& c, float radius, float angl, int count) {
	float a = TsUtil::RandRange(0.7f, 1.2f);
	float r = radius * a;
	Circle cc (	c + c.r * TsUtil::SinCosPos(angl), r, N * a);
	mCircles.Add(cc);
	if ( count > 0 ) {
		angl += (count & 2) ?	TsUtil::RandRange(-40, 40) :
				(count & 1) ?	TsUtil::RandRange(-40,-20) :
								TsUtil::RandRange( 20, 40) ;
		CreateChild(cc, radius, angl, count - 1);
	}
}

void	TsBiomeSite::Generate(float _x, float _y, float radius)
{
	mX = _x;
	mY = _y;

	{// create main circles -----------------------------------------------------------------
		float ra = TsUtil::RandRange(1.0f, 1.5f);
		Circle center(FVector2D(mX, mY), radius * ra, N * ra);
		mCircles.Add(center);

		float angl = 0;
		CreateChild(center, radius, angl, TsUtil::RandRange(1, 2));
		angl += TsUtil::RandRange(90.0f, 120.0f);
		CreateChild(center, radius, angl, TsUtil::RandRange(1, 2));
		angl += TsUtil::RandRange(90.0f, 120.0f);
		CreateChild(center, radius, angl, TsUtil::RandRange(1, 2));
	}
}

void	TsBiomeSite::Release()
{
	mCircles.Empty();
}


void	TsBiomeSite::UpdateBoundingbox( FBox2D & boundingbox )
{// create boundingbox -------------------------------------------------------------------
	boundingbox = FBox2D(FVector2D(10000, 10000), FVector2D(-10000, -10000));
	for (auto& c : mCircles) {
		for (int i = 0; i < c.n; i++) {
			boundingbox += c.GetOutline(i + 0);
			boundingbox += c.GetOutline(i + 1);
		}
	}
	FVector2D size = boundingbox.GetSize();
	if (size.X > size.Y) boundingbox.Max.X += size.X - size.Y;
	if (size.Y > size.X) boundingbox.Max.Y += size.Y - size.X;
}

float	 TsBiomeSite::GetMaterialValue(const FVector2D& p)
{
	float h = 1;
	for (auto& c : mCircles) {
		if ( c.IsInside(p) ) return 0;

		float hp = 1;
		for (int i = 0; i < c.n; i++) {
			FVector2D v0 = c.GetOutline(i + 0);
			FVector2D v1 = c.GetOutline(i + 1);
			hp = FMath::Min( (TsUtil::NearPoint(v0, v1, p) - p).Length() / 50.0f, hp ) ;
		}
		h = FMath::Min(h, hp);
	}
	return h;
}

float	TsBiomeSite::Remap(float val) const
{
	return 100 * (FMath::Pow(TsValueMap::Remap(val), 2.0f ) - 1 );
}

//float	 TsBiomeSite::GetValue(const FVector2D &p )
//{
//	return	( 1.00f * FMath::PerlinNoise2D(0.001f * mN0 * p + mNoisePos)
//			+ 0.50f * FMath::PerlinNoise2D(0.002f * mN0 * p + mNoisePos)
//			+ 0.25f * FMath::PerlinNoise2D(0.004f * mN0 * p + mNoisePos)
//			+ 0.13f * FMath::PerlinNoise2D(0.008f * mN0 * p + mNoisePos)
//			+ 0.06f * FMath::PerlinNoise2D(0.016f * mN0 * p + mNoisePos)
//			+ 0.03f * FMath::PerlinNoise2D(0.032f * mN0 * p + mNoisePos));
//#if 0
//	float h = -1000000;
//	for (auto& c : mCircles) {
//		if ( c.IsInside( p ) ){
//			h = 1.0f;
//		} else {
//			float hc = -1000000.0f;
//			for (int i = 0; i < c.n; i++) {
//				FVector2D v0 = c.GetOutline(i + 0);
//				FVector2D v1 = c.GetOutline(i + 1);
//				float hp = 1.0f - (TsUtil::NearPoint(v0, v1, p) - p).Length() / 10;
//				hc = FMath::Max(hc, hp) ;
//			}
//			h = FMath::Max(h, hc);
//		}
//	}
//	return h;
//#endif
//}

TsBiomeSite::TsBiomeSite()
	: TsBiomeSFunc(
		TsNoiseParam({
			{ 1.00f, 0.001f },
			{ 0.50f, 0.002f },
			{ 0.25f, 0.004f },
			{ 0.13f, 0.008f },
			{ 0.06f, 0.016f },
			{ 0.03f, 0.032f },
		}), 1.0f ){}

	//for (uint32 RegionIndex = 0; RegionIndex < region_data->NumRegions; ++RegionIndex) {
	//	int32 CurrentFirstMip = region_data->Texture2DResource->GetCurrentFirstMip();
	//	if (region_data->MipIndex >= CurrentFirstMip) {
	//		RHIUpdateTexture2D(
	//			region_data->Texture2DResource->GetTexture2DRHI(),
	//			region_data->MipIndex - CurrentFirstMip,
	//			region_data->Regions[RegionIndex],
	//			region_data->SrcPitch,
	//			region_data->SrcData 
	//				+ region_data->Regions[RegionIndex].SrcY * region_data->SrcPitch 
	//				+ region_data->Regions[RegionIndex].SrcX * region_data->SrcBpp
	//		);
	//	}
	//}
	//if (bFreeData) {
	//	FMemory::Free(region_data->Regions);
	//	FMemory::Free(region_data->SrcData);
	//}



bool	TsBiomeSite::IsInside( const FVector2D &p ) 
{
	for (auto& c : mCircles) {
		if ( c.IsInside( p ) ) return true;
	}
	return false;
}

//------------------------------------------------------- Debug

void	TsBiomeSite::Debug(UWorld* world) {
	for (auto c : mCircles) {
#define UP 1.0f
		DrawDebugCircle(world, FVector(c.X, c.Y, UP), 10, 32, FColor(255, 0, 255), true, 10000);
		for (int i = 0; i < c.n; i++) {
			FVector	p0 = FVector( c.GetOutline(i + 0), UP);
			FVector	p1 = FVector( c.GetOutline(i + 1), UP);
			DrawDebugLine(world, p0, p1, FColor(255, 0, 0), true, 10000);
		}
	}
}

