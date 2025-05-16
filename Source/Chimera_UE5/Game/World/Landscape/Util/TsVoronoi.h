#pragma once

#include <functional>
#include "CoreMinimal.h"


// -------------------------------- TsVoronoi  --------------------------------
//
//
struct TsVoronoi
	: public FVector2D		// center position
{
public:
	enum EFlag {
		None,
		IsVoronoi    = 1UL << 0,
		IsBound      = 1UL << 2,

		HasSameBiome = 1UL << 3,
		HasReference = 1UL << 4,// this data is referenced.
	};
	struct Point : public FVector2D {
		int			mID_a, mID_b;
		int			mFlag;
		Point() : FVector2D(0, 0), mID_a(0), mID_b(0), mFlag(EFlag::None) {}
		Point( FVector2D _p, int _a, int _b, int f ) : FVector2D(_p), mID_a(_a), mID_b(_b), mFlag(f) {}

		bool operator == (const Point& a) const { // 
			return (X == a.X && Y == a.Y) && ((mID_a == a.mID_a && mID_b == a.mID_b) || (mID_a == a.mID_b && mID_b == a.mID_a));
		}
	};

	struct Edge {
		FVector2D	mP, mD;
		int			mFlag;
		int			mID ;
		TsVoronoi*	mShared ;// the edge is shared with other TsVoronoi

		Edge() : mP(0, 0), mD(0, 0), mFlag(EFlag::None), mID(0), mShared(nullptr) {}
		Edge(const FVector2D& p0, const FVector2D& p1, int f = EFlag::None);
		Edge(float x, float y, float _dx, float _dy, int f = EFlag::None);

		//static float _Intersect(const Edge& a, const Edge& b) const ;
		bool		Intersect(const Edge& a, FVector2D& point) const ;

		bool operator == (const Edge& a) const		{ return mP == a.mP && mD == a.mD; }

		FVector2D	GetPoint(float d) const			{ return mP + d * mD; }
		float		Cross(const FVector2D& v) const	{ return mD.X * (v.Y - mP.Y) - mD.Y * (v.X - mP.X); }
		void		SetVoronoi(TsVoronoi* ed)	{ mShared = ed; }
		float		GetDistance(const FVector2D& p) const ;
	};

public:
	TArray<Edge>		mEdges;
	FVector2D			mMin, mMax;
	void*				mOwner ;
public:
	TsVoronoi()
		: FVector2D(0, 0)
		, mMin( 1000000.0f,  1000000.0f)
		, mMax(-1000000.0f, -1000000.0f)
		, mOwner(nullptr) {}

	TsVoronoi(float x, float y, void *owner)
		: FVector2D(x, y)
		, mMin( 1000000.0f,  1000000.0f)
		, mMax(-1000000.0f, -1000000.0f)
		, mOwner(owner) {}

	bool		IsInside(const FVector2D& p) const ;

	void		AddEdge(const Edge& edge);
	void		ForeachEdge( std::function< void(const Edge &) >);

public:
	void		Debug(UWorld* world, const FColor &c) const ;
} ;



// -------------------------------- TsVoronoiSite  --------------------------------
//
//

template <typename T>
concept DerivedFromTsVoronoi = std::is_base_of_v<TsVoronoi, T>;

template <DerivedFromTsVoronoi T>
struct TsVoronoiSite
{
private:
	TArray<TsVoronoi::Edge >	mBoundedges;
	TArray<TsVoronoi::Point>	mBoundpoints;
public:

	void GenerateSite(
			TArray<T>&		voronois,
			const FBox2D &	boundary,
			float			voronoi_size,
			float			voronoi_jitter
		)	
	{

		UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Create Voronoi"));

		{//-------------------------------------------- boundary
			FVector2D
				bp00(boundary.Min.X, boundary.Min.Y), bp10(boundary.Max.X, boundary.Min.Y),
				bp01(boundary.Min.X, boundary.Max.Y), bp11(boundary.Max.X, boundary.Max.Y);
			TsVoronoi::Edge
				be0(bp00, bp01, TsVoronoi::EFlag::IsBound), be1(bp01, bp11, TsVoronoi::EFlag::IsBound),
				be2(bp11, bp10, TsVoronoi::EFlag::IsBound), be3(bp10, bp00, TsVoronoi::EFlag::IsBound);

			mBoundedges.Add(be0);
			mBoundedges.Add(be1);
			mBoundedges.Add(be2);
			mBoundedges.Add(be3);
			mBoundpoints.Add(TsVoronoi::Point(bp00, be0.mID, be3.mID, TsVoronoi::EFlag::IsBound));
			mBoundpoints.Add(TsVoronoi::Point(bp01, be0.mID, be1.mID, TsVoronoi::EFlag::IsBound));
			mBoundpoints.Add(TsVoronoi::Point(bp11, be1.mID, be2.mID, TsVoronoi::EFlag::IsBound));
			mBoundpoints.Add(TsVoronoi::Point(bp10, be2.mID, be3.mID, TsVoronoi::EFlag::IsBound));
		}

		{//-------------------------------------------- generate random site.
			for (float x = boundary.Min.X; x < boundary.Max.X; x += voronoi_size) {
				for (float y = boundary.Min.Y; y < boundary.Max.Y; y += voronoi_size) {
					float px = x + FMath::RandRange(-voronoi_jitter, voronoi_jitter) * voronoi_size;
					float py = y + FMath::RandRange(-voronoi_jitter, voronoi_jitter) * voronoi_size;

					if ( boundary.IsInside(FVector2D(px, py)) ) {
						voronois.Add(T(px, py));
					}
				}
			}
		}

	}
	void Tesselate(TArray<T>& voronoi_list ){
		UE_LOG(LogTemp, Log, TEXT("UTsLandscape::    tesselate all  Voronoi-sites"));

		//-------------------------------------------- tesselate all sites
		for (auto& v : voronoi_list) {
			TArray<T> voronois = voronoi_list;
			// sort by the distance of the each voronoi-sites.
			voronois.Sort([v](const TsVoronoi& a, const TsVoronoi& b) { return (a - v).Length() < (b - v).Length(); });

			// init by boundary edges
			TArray<TsVoronoi::Edge>		edges (mBoundedges ) ;
			TArray<TsVoronoi::Point>	points(mBoundpoints) ;

			int vertnum = 0;
			// under go with all voronoi-sites.
			for (auto& c : voronois) {
				if (c == v) continue;

				TsVoronoi::Edge edge((c.X + v.X) / 2, (c.Y + v.Y) / 2, (v.Y - c.Y), (c.X - v.X), TsVoronoi::EFlag::IsVoronoi);
				edges.Add(edge);

				if (edges.Num() > 1) {
					for (auto& e : edges) {							// add cross-point of new edges
						FVector2D pos;
						if (edge.Intersect(e, pos)) {
							points.AddUnique(TsVoronoi::Point(pos, e.mID, edge.mID, edge.mFlag | e.mFlag));
						}
					}
					for (auto& p : points) {						// check the point is inside of edge
						if (p.mID_a < 0 || p.mID_b < 0) continue;	// skip invalid edges
						for (auto& e : edges) {
							if (e.mID == p.mID_a || e.mID == p.mID_b) continue;

							auto cross = [&](const FVector2D& pn, const TsVoronoi::Edge& ed) { // cross product
								return ed.mD.X * (pn.Y - ed.mP.Y) - ed.mD.Y * (pn.X - ed.mP.X);
								};
							float d0 = cross(p, e);
							float d1 = cross(v, e);
							if (cross(p, e) * cross(v, e) < 0) {	// it is different side of the edge.
								p.mID_a = p.mID_b = -1;				// invalidate the point
								break;
							}
						}
					}
					for (auto& e : edges) {						// count reference of the edge
						e.mFlag &= ~TsVoronoi::EFlag::HasReference;
						for (auto p : points) {
							if (p.mID_a == e.mID) e.mFlag |= TsVoronoi::EFlag::HasReference;
							if (p.mID_b == e.mID) e.mFlag |= TsVoronoi::EFlag::HasReference;
						}
					}

					// remove all invalid points and edges.
					points.RemoveAll([](const TsVoronoi::Point& a) { return a.mID_a < 0 || a.mID_b < 0; });
					edges.RemoveAll([](const TsVoronoi::Edge& a) { return !(a.mFlag & TsVoronoi::EFlag::HasReference); });

					if (++vertnum > 10) break;///////////// this is maybe wrong.... what would be 
				}
			}

			{// refine the edge
				for (auto& e : edges) {
					TsVoronoi::Point pp[2];
					int        pi = 0;
					for (auto& p : points) {
						if (p.mID_a == e.mID || p.mID_b == e.mID) pp[pi++] = p;
						if (pi == 2) break;
					}
					if (pi == 1) {
						pp[1] = TsVoronoi::Point(pp[0] + FVector2D(e.mD.X, e.mD.Y) * 2, e.mID, 0, e.mFlag);
					}

					TsVoronoi::Edge	e01(pp[0], pp[1], e.mFlag);
					TsVoronoi::Edge	e10(pp[1], pp[0], e.mFlag);
					v.AddEdge(TsVoronoi::Edge(e01.Cross(v) < 0.0f ? e10 : e01));
				}
			}

			{// adjacent voronoi
				for (auto& ea : v.mEdges) {
					for (auto& vb : voronoi_list) {
						if (v == vb) continue;
						bool done = false;
						for (auto& eb : vb.mEdges) {
							if (ea.mID == eb.mID) {
								ea.mShared = &vb;
								eb.mShared = &v;
								done = true;
								break;
							}
						}
						if (done) break;
					}
				}
			}
		}

		UE_LOG(LogTemp, Log, TEXT("UTsLandscape:: Voronoi done."));

		//-------------------------------------------------------------------------------------- create voronois, done.

	}
};
