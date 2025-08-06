#pragma once

#include "CoreMinimal.h"

#include "TsBiomeSurface.h"


// determine the shape of the Site itself.

class TsBiomeSite
	: public TsBiomeSFunc
{
public:
	struct CircleConf{
		FVector2D	mRRange;	//1.0f, 1.5f
		FVector2D	mRNoise;	//0.06f 0.25f
		int			mChildren ;
		FVector2D	mARange;	//1.0f, 1.5f

		CircleConf(float cr_mn, float cr_mx, float cn_x, float cn_y, int num , float a_mn, float a_mx )
		: mRRange(cr_mn, cr_mx), mRNoise(cn_x,cn_y), mChildren(num), mARange(a_mn,a_mx){}
	} ;

	struct Circle : public FVector2D {
		float		mR;			// radius
		int			mN;			// subdiv
		int			mLevel;		// level

		float		mNoiseScale ;			// noise scale
		float		mNoiseRate  ;			// noise rate

		Circle(const FVector2D &v, float r=0, int n=0, int l=0, float ns=0.06f, float nr = 0.5f)
			: FVector2D(v), mR(r), mN(n), mLevel(l), mNoiseScale(ns), mNoiseRate(nr) {}

		bool		IsInside( const FVector2D& p ) const ;
		FVector2D	GetOutline(int i) const ;
	};

	float			mX, mY;
	TArray<Circle>	mCircles;	// 


	//------------------------------------------------------- Island
private:
	void			CreateChild( const Circle& c, float radius, float angl, TArray<CircleConf> conf );
public:
	TsBiomeSite();
	virtual			~TsBiomeSite() {}


	void			Generate( const FVector2D& p, float radius, std::initializer_list<CircleConf> conf_list ){
		Generate( p.X, p.Y, radius, conf_list );
	}
	void			Generate( float _x, float _y, float radius, std::initializer_list<CircleConf> conf_list );

	void			UpdateBoundingbox( FBox2D& bounding );
	bool			IsInside( const FVector2D& p )const ;

	TArray<Circle>	FindCircle( int level ){
		TArray<Circle>	list ;
		for ( const auto &c : mCircles ) if ( c.mLevel==level ) list.Add( c ) ;
		return			list;
	}
	float			GetMaterialValue(const FVector2D& p);
	float			Remap(float val) const override;

	//------------------------------------------------------- Debug
	void			Debug(UWorld* world);
};

