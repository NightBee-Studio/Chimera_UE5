
#include "TsBiomeSite.h"
#include "../Util/TsUtility.h"




// ----------------------------- Circle ------------------------------
bool	TsBiomeSite::Circle::IsInside( const FVector2D& p ) const {
	int cn = 0;											// the  crossing number counter
	for ( int i=0 ; i<mN ; i++ ) {						// loop through all edges of the polygon
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

FVector2D TsBiomeSite::Circle::GetOutline(int i) const {
	//const float S = 0.06f ;
	float ang  = (i % mN) * 360.0f / mN;
	float bias = FMath::PerlinNoise3D( FVector(X, Y, ang * mNoiseScale) );
	float l = (1 + bias * mNoiseRate) * mR;
	return *this + l * TsUtil::SinCosPos(ang);
}



void	TsBiomeSite::CreateChild(  const Circle& cen, float radius, float angl, TArray<CircleConf> conf )
{
	const int N = 128 ;

	CircleConf cnf = conf[0] ;
	conf.RemoveAt( 0 ) ;

	float ra = TsUtil::RandRange( cnf.mRRange );
	Circle c ( cen + cen.mR * TsUtil::SinCosPos(angl), radius * ra, N * ra, conf.Num(), cnf.mRNoise.X, cnf.mRNoise.Y );
	mCircles.Add( c );
	if ( conf.Num() > 0 ) {
		angl += TsUtil::RandRange( cnf.mARange ) ;
		if ( cnf.mChildren > 0 ){
			float min = 360/ (cnf.mChildren+1) ;
			float max = 360/ (cnf.mChildren+0) ;
			for ( int i=0 ; i<cnf.mChildren ; i++ ){
				CreateChild( c, radius, angl, conf );
				angl += TsUtil::RandRange(min, max);
			}
		}
	}
}

void	TsBiomeSite::Generate(float _x, float _y, float radius, std::initializer_list<CircleConf> conf_list )
{
	mX = _x;
	mY = _y;

	TArray<CircleConf> conf(conf_list);
	CreateChild( Circle(FVector2D(mX, mY)), radius, 0.0f, conf);
}


void	TsBiomeSite::UpdateBoundingbox( FBox2D & boundingbox )
{// create boundingbox -------------------------------------------------------------------
	boundingbox = FBox2D(FVector2D(10000, 10000), FVector2D(-10000, -10000));
	for (auto& c : mCircles) {
		for (int i = 0; i < c.mN; i++) {
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
		for (int i = 0; i < c.mN; i++) {
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


bool	TsBiomeSite::IsInside( const FVector2D &p ) const
{
	for (auto& c : mCircles) {
		if ( c.IsInside( p ) ) return true;
	}
	return false;
}

//------------------------------------------------------- Debug

void	TsBiomeSite::Debug(UWorld* world) {
	const float UP = 1.0f;
	for (auto c : mCircles) {
		DrawDebugCircle(world, FVector(c.X, c.Y, UP), 10, 32, FColor(255, 0, 255), true, 10000, 0, 2.0f, FVector::XAxisVector,FVector::YAxisVector, false );
		DrawDebugCircle(world, FVector(c.X, c.Y, UP), c.mR, 32, FColor(0,0,255), true, 10000, 0, 2.0f, FVector::XAxisVector,FVector::YAxisVector, false  );
		//for (int i = 0; i < c.mN; i++) {
		//	FVector	p0 = FVector( c.GetOutline(i + 0), UP);
		//	FVector	p1 = FVector( c.GetOutline(i + 1), UP);
		//	DrawDebugLine(world, p0, p1, FColor(255, 0, 0), true, 10000);
		//}
	}
}

