#pragma once

#include "CoreMinimal.h"
#include "TsBuilder.generated.h"



// -------------------------------- UTsLandscape  --------------------------------
UCLASS(Blueprintable)
class CHIMERA_UE5_API ATsBuilder
	: public AActor
{
	GENERATED_BODY()

private:
	void*		mImplement ;

public:
	ATsBuilder();

	UFUNCTION(BlueprintCallable)
	void		Build(
		float _x, float _y, float radius,
		float	voronoi_size,
		float	voronoi_jitter,
		int		heightmap_reso,
		int		erode_cycle);
} ;


