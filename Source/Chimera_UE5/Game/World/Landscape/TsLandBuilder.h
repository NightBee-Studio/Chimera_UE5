#pragma once

#include "CoreMinimal.h"
#include "TsLandBuilder.generated.h"



// -------------------------------- UTsLandscape  --------------------------------
UCLASS(Blueprintable)
class CHIMERA_UE5_API UTsLandBuilder
	: public UObject
{
	GENERATED_BODY()

private:
	void*		mImplement ;

public:
	UTsLandBuilder();

	void		Build(
		float _x, float _y, float radius,
		float	voronoi_size,
		float	voronoi_jitter,
		int		heightmap_reso,
		int		erode_cycle);

};

