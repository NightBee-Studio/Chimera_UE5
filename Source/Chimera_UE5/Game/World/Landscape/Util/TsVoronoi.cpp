

#include "TsVoronoi.h"
#include "TsUtility.h"


// -------------------------------- ID --------------------------------
static int gen_voronoi_id(float x0, float y0, float x1 = 0, float y1 = 0) {
	unsigned int code =
		0x0fffffff & (unsigned int)(
			(int)(FMath::Min(x0, x1) + 0.5f)
			+ (int)(FMath::Max(x0, x1) + 0.5f) * 10
			+ (int)(FMath::Min(y0, y1) + 0.5f) * 20
			+ (int)(FMath::Max(y0, y1) + 0.5f) * 200
			);
	return (int)code + 1;
}

// -------------------------------- Edge  --------------------------------
//
//

TsVoronoi::Edge::Edge(const FVector2D& p0, const FVector2D& p1, int f)
	: mP(p0 ), mD(p1 - p0), mFlag(f)
	, mID(gen_voronoi_id(p0.X, p0.Y, p1.X, p1.Y))	// ID will be calced by position
	, mShared(nullptr) {}

TsVoronoi::Edge::Edge(float x, float y, float dx, float dy, int f)
	: mP(x,y), mD(dx, dy), mFlag(f)
	, mID(gen_voronoi_id(x, y, x+dx, y+dy))			// ID will be calced by position
	, mShared(nullptr) {}


float intersect(const TsVoronoi::Edge& a, const TsVoronoi::Edge& b)
{
	float delta = a.mD.X * b.mD.Y - a.mD.Y * b.mD.X;
	if (delta != 0.0f) {
		return (b.mD.X * (a.mP.Y - b.mP.Y) - b.mD.Y * (a.mP.X - b.mP.X)) / delta;
	}
	return 0.0f;
}

bool TsVoronoi::Edge::Intersect(const Edge& a, FVector2D& point) const 
{ 
	float d = intersect(*this, a);
	if (d != 0.0f) {
		point = GetPoint(d);
		return true;
	}
	return false;
}

float	TsVoronoi::Edge::GetDistance(const FVector2D& p) const 
{
	return (TsUtil::NearPoint(mP, mP + mD, p) - p).Length();
}



// -------------------------------- TsVoronoi  --------------------------------
//
//

bool	TsVoronoi::IsInside(const FVector2D& p ) const
{
	auto cross = [&](const Edge& e) {
		return e.mD.X * (p.Y - e.mP.Y) - e.mD.Y * (p.X - e.mP.X);
	} ;
	if (mEdges.Num() == 0) return false;
	float dd = 0;
	for ( auto e : mEdges ) {
		if      (dd == 0.0f) dd = cross(e);
		else if (dd * cross(e) <= 0.0f) return false;
	}
	return true;
}

void	TsVoronoi::AddEdge(const Edge& edge)
{
	mMin = FVector2D(
		FMath::Min(edge.mP.X, mMin.X),
		FMath::Min(edge.mP.Y, mMin.Y)
	);
	mMax = FVector2D(
		FMath::Max(edge.mP.X, mMax.X),
		FMath::Max(edge.mP.Y, mMax.Y) 
	);
	mEdges.Add(edge);
}

void	TsVoronoi::ForeachEdge( std::function< void(const Edge &) > func)
{
	for (const auto &e : mEdges) {
		func( e );
	}
}

void	TsVoronoi::Debug(UWorld* world, const FColor& c ) const {
#define UP 1.0f
	DrawDebugCircle(world, FVector(X, Y, UP), 10, 8, FColor(155, 0, 0), true, 10000);
	for (auto e : mEdges) {
		FVector p0 = FVector((*this) * 0.05f + (e.mP       ) * 0.95f, UP);
		FVector p1 = FVector((*this) * 0.05f + (e.mP + e.mD) * 0.95f, UP);
		DrawDebugLine(world, p0, p1, c, true, 10000);
	}
}

