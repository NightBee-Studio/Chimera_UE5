#pragma once

#include "CoreMinimal.h"
#include "TsTextureMap.h"
#include "TsUtility.h"

class TsHeightMesh 
{
public:

	static 
	void Build(
		TsTextureMap*			tex_map ,
		const TsUtil::TsBox &	tex_rect,
		int						mesh_div,
		float					mesh_size,
		float					mesh_height,
		const FString&			assetname );
};

