#pragma once

#include "../TsLandSurface.h"


class SurfaceMountain : public TsBiomeSrfFunc {
private:
	float				mPowerFactor ;
public:
	static
	SurfaceMountain*	gInstance;
	static
	TsNoiseMap *		gNoiseMap;

public:
	SurfaceMountain(const TsNoiseParam& cnf, float h, float pw = 2.0f)
		: TsBiomeSrfFunc(cnf, h), mPowerFactor(pw) {
		gInstance = this;
	}

	float	GetMountValue(const FVector2D& p) ;
	float	GetValue( const FVector2D& p ) override;
	float	Remap(float val) const override;

	void	Exec_EachGroup( TsBiomeGroup& grp ) override;
	void	Exec_UpdateRemap(const FVector2D& p) override;

	static
	void	SetNoiseConfig( const TsNoiseParam&cnf ){
		gNoiseMap = new TsNoiseMap(cnf);
	}

	static
	void	GetMountains(TArray<TsVoronoi>& v_list);

	static
	void	CreateMountainA(FVector2D pos, float radius, int branch_max = 3);
	static
	void	CreateMountainB(TArray<TsBiome*>&list);

public:
	static void Debug(UWorld* world);
};
