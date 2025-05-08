#pragma once

#include "CoreMinimal.h"


// -------------------------------- TsVoronoi  --------------------------------
//
//
struct TsVoronoi
	: public FVector2D
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

		static float _Intersect(Edge& a, Edge& b);
		bool		Intersect(Edge& a, FVector2D& point);

		bool operator == (const Edge& a) const	{ return mP == a.mP && mD == a.mD; }

		FVector2D	GetPoint(float d) const		{ return mP + d * mD; }
		float		Cross(const FVector2D& v)	{ return mD.X * (v.Y - mP.Y) - mD.Y * (v.X - mP.X); }
		void		SetVoronoi(TsVoronoi* ed)	{ mShared = ed; }
	};

public:
	TArray<Edge>		mEdges;
	FVector2D			mMin, mMax;

public:
	TsVoronoi()
		: FVector2D(0, 0)
		, mMin( 1000000.0f,  1000000.0f)
		, mMax(-1000000.0f, -1000000.0f) {}
	TsVoronoi(float x, float y)
		: FVector2D(x, y)
		, mMin( 1000000.0f,  1000000.0f)
		, mMax(-1000000.0f, -1000000.0f) {}

	int			GetOwnerD() const ;
	bool		IsInside(const FVector2D& p) const ;

	void		AddEdge(const Edge& edge);

public:
	void		Debug(UWorld* world, const FColor &c) const ;
} ;



// -------------------------------- TsVoronoiSite  --------------------------------
//
//
struct TsVoronoiSite
{
	
	void Generate() {}
	void Tesselate() {}
};
