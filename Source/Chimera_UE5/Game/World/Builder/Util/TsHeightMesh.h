#pragma once

#include "CoreMinimal.h"

#include "../Biome/TsBiomeMap.h"
#include "TsUtility.h"


class UMaterialInterface;
class TsHeightMesh 
{
public:
	static 
	void Build(
			TsBiomeMap*			tex_map ,
			const TsUtil::TsBox &	tex_rect,
			TsNoiseMap *			noise_map,
			float					noise_scale,
			int						mesh_div,
			float					mesh_size,
			float					mesh_height,
			const FString&			assetname,
			UMaterialInterface*		material = nullptr
		);
};

