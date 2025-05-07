#pragma once

#include "CoreMinimal.h"

#include "TsImageMap.h"




class TsLandShape
	: public TsValueMap
{
private:
	struct Circle : public FVector2D {
		float		r;			// radius
		int			n;			// subdiv

		Circle(const FVector2D &_v, float _r, int _n)
			: FVector2D(_v), r(_r), n(_n) {}

		bool		IsInside( const FVector2D& p );
		FVector2D	GetOutline(int i);
	};

public:
	float			mX, mY;
	TArray<Circle>	mCircles;	// 

	//------------------------------------------------------- Island
private:
	void			CreateChild(const Circle& c, float radius, float angl, int count);

public:
	TsLandShape() : TsValueMap(){}
	virtual			~TsLandShape() {}

	void			Generate(float _x, float _y, float radius);
	void			Release();
	void			UpdateBoundingbox(FBox2D& boundingbox);

	bool			IsInside( const FVector2D& p );

	float			GetMaterialValue(const FVector2D& p);

	float			Remap(float val) const override;
	float			GetValue(const FVector2D& p) override ;

	//------------------------------------------------------- Debug
	void			Debug(UWorld* world);
};

