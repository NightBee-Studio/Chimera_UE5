
#include "TsLandShape.h"

#include "CoreMinimal.h"



// ---------------------------- Utillity -------------------------

// nearest point 
static FVector2D near_point(const FVector2D& v1, const FVector2D& v2, const FVector2D& pt)
{
	FVector2D ap = pt - v1;
	FVector2D ab = v2 - v1;
	if (ap.Dot(ab) < 0) return v1;
	FVector2D bp = pt - v2;
	FVector2D ba = v1 - v2;
	if (bp.Dot(ba) < 0) return v2;
	return v1 + ab / ab.Length() * (ap.Dot(ab) / ab.Length());
}

// ID 
static int gen_pos_id(float x, float y, float dx = 0, float dy = 0) {
	unsigned int code = 0x0fffffff & (unsigned int)(x + 100 * y + dx + dy * 20);
	return (int)code + 1;
}

static float sigmoid(float x) {
	return tanh(x / 2) ;
//	return (tanh(x/2) + 1)/2 - 0.5f ;
}

static inline FVector2D sincos_pos(float ang) {
	float x, y ;
	FMath::SinCos( &y, &x, FMath::DegreesToRadians(ang));
	return FVector2D(x,y);
}




// ----------------------------- Circle ------------------------------
bool	TsLandShape::Circle::IsInside( const FVector2D& p ) {
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
FVector2D TsLandShape::Circle::GetOutline(int i) {
	float ang  = (i % n) * 360.0f / n;
	float bias = FMath::PerlinNoise3D( FVector(X, Y, ang * 0.06f) );
	float l = (1 + bias * 0.5f) * r;
	return *this + l * sincos_pos(ang);
}

#define N 128
void	TsLandShape::CreateChild(  const Circle& c, float radius, float angl, int count) {
	float a = FMath::RandRange(0.7f, 1.2f);
	float r = radius * a;
	Circle cc (	c + c.r * sincos_pos(angl), r, N * a);
	mCircles.Add(cc);
	if ( count > 0 ) {
		angl += (count & 2) ?	FMath::RandRange(-40, 40) :
				(count & 1) ?	FMath::RandRange(-40,-20) :
								FMath::RandRange( 20, 40) ;
		CreateChild(cc, radius, angl, count - 1);
	}
}

void	TsLandShape::Generate(float _x, float _y, float radius)
{
	mX = _x;
	mY = _y;

	{// create main circles -----------------------------------------------------------------
		float ra = FMath::RandRange(1.0f, 1.5f);
		Circle center(FVector2D(mX, mY), radius * ra, N * ra);
		mCircles.Add(center);

		float angl = 0;
		CreateChild(center, radius, angl, FMath::RandRange(1, 2));
		angl += FMath::RandRange(90.0f, 120.0f);
		CreateChild(center, radius, angl, FMath::RandRange(1, 2));
	}


}

void	TsLandShape::Release()
{
	mCircles.Empty();
}


void	TsLandShape::UpdateBoundingbox( FBox2D & boundingbox )
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

float	 TsLandShape::GetMaterialValue(const FVector2D& p)
{
	float h = 1;
	for (auto& c : mCircles) {
		if ( c.IsInside(p) ) return 0;

		float hp = 1;
		for (int i = 0; i < c.n; i++) {
			FVector2D v0 = c.GetOutline(i + 0);
			FVector2D v1 = c.GetOutline(i + 1);
			hp = FMath::Min( (near_point(v0, v1, p) - p).Length() / 50.0f, hp ) ;
		}
		h = FMath::Min(h, hp);
	}
	return h;
}


float	TsLandShape::Remap(float val) const
{
	return 100 * (FMath::Pow(TsValueMap::Remap(val), 2.0f ) - 1 );
}

float	 TsLandShape::GetValue(const FVector2D &p )
{
	float h = -1000000;
	for (auto& c : mCircles) {
		if ( c.IsInside( p ) ){
			h = 1.0f;
		} else {
			float hc = -1000000.0f;
			for (int i = 0; i < c.n; i++) {
				FVector2D v0 = c.GetOutline(i + 0);
				FVector2D v1 = c.GetOutline(i + 1);
				float hp = 1.0f - (near_point(v0, v1, p) - p).Length() / 10;
				hc = FMath::Max(hc, hp) ;
			}
			h = FMath::Max(h, hc);
		}
	}
	return h;
}

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



bool	TsLandShape::IsInside( const FVector2D &p ) 
{
	for (auto& c : mCircles) {
		if ( c.IsInside( p ) ) return true;
	}
	return false;
}

//------------------------------------------------------- Debug

void	TsLandShape::Debug(UWorld* world) {
	for (auto c : mCircles) {
		DrawDebugCircle(world, FVector(c.X, c.Y, 0), 10, 32, FColor(255, 0, 255), true, 10000);
		for (int i = 0; i < c.n; i++) {
			FVector2D	p0 = c.GetOutline(i + 0);
			FVector2D	p1 = c.GetOutline(i + 1);
			DrawDebugLine(world, FVector(p0.X, p0.Y, 0), FVector(p1.X, p1.Y, 0), FColor(255, 0, 0), true, 10000);
		}
	}
}

